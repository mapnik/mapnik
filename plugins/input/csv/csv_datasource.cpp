/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#include "csv_datasource.hpp"
#include "csv_utils.hpp"

// boost
#include <boost/make_shared.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/memory_featureset.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/json/geometry_parser.hpp>
#include <mapnik/util/geometry_to_ds_type.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/boolean.hpp>

// stl
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

using mapnik::datasource;
using mapnik::parameters;
using namespace boost::spirit;

DATASOURCE_PLUGIN(csv_datasource)

csv_datasource::csv_datasource(parameters const& params, bool bind)
    : datasource(params),
      desc_(*params_.get<std::string>("type"), *params_.get<std::string>("encoding", "utf-8")),
      extent_(),
      filename_(),
      inline_string_(),
      file_length_(0),
      row_limit_(*params_.get<int>("row_limit", 0)),
      features_(),
      escape_(*params_.get<std::string>("escape", "")),
      separator_(*params_.get<std::string>("separator", "")),
      quote_(*params_.get<std::string>("quote", "")),
      headers_(),
      manual_headers_(boost::trim_copy(*params_.get<std::string>("headers", ""))),
      strict_(*params_.get<mapnik::boolean>("strict", false)),
      quiet_(*params_.get<mapnik::boolean>("quiet", false)),
      filesize_max_(*params_.get<float>("filesize_max", 20.0)),  // MB
      ctx_(boost::make_shared<mapnik::context_type>())
{
    /* TODO:
       general:
       - refactor parser into generic class
       - tests of grid_renderer output
       - ensure that the attribute desc_ matches the first feature added
       alternate large file pipeline:
       - stat file, detect > 15 MB
       - build up csv line-by-line iterator
       - creates opportunity to filter attributes by map query
       speed:
       - add properties for wkt/json/lon/lat at parse time
       - add ability to pass 'filter' keyword to drop attributes at layer init
       - create quad tree on the fly for small/med size files
       - memory map large files for reading
       - smaller features (less memory overhead)
       usability:
       - enforce column names without leading digit
       - better error messages (add filepath) if not reading from string
       - move to spirit to tokenize and add character level error feedback:
       http://boost-spirit.com/home/articles/qi-example/tracking-the-input-position-while-parsing/
    */

    boost::optional<std::string> inline_string = params_.get<std::string>("inline");
    if (inline_string)
    {
        inline_string_ = *inline_string;
    }
    else
    {
        boost::optional<std::string> file = params_.get<std::string>("file");
        if (!file) throw mapnik::datasource_exception("CSV Plugin: missing <file> parameter");

        boost::optional<std::string> base = params_.get<std::string>("base");
        if (base)
            filename_ = *base + "/" + *file;
        else
            filename_ = *file;
    }

    if (bind)
    {
        this->bind();
    }
}


csv_datasource::~csv_datasource() { }

void csv_datasource::bind() const
{
    if (is_bound_) return;

    if (!inline_string_.empty())
    {
        std::istringstream in(inline_string_);
        parse_csv(in,escape_, separator_, quote_);
    }
    else
    {
        std::ifstream in(filename_.c_str(),std::ios_base::in | std::ios_base::binary);
        if (!in.is_open())
            throw mapnik::datasource_exception("CSV Plugin: could not open: '" + filename_ + "'");
        parse_csv(in,escape_, separator_, quote_);
        in.close();
    }
    is_bound_ = true;
}

template <typename T>
void csv_datasource::parse_csv(T & stream,
                               std::string const& escape,
                               std::string const& separator,
                               std::string const& quote) const
{
    stream.seekg(0, std::ios::end);
    file_length_ = stream.tellg();

    if (filesize_max_ > 0)
    {
        double file_mb = static_cast<double>(file_length_)/1048576;

        // throw if this is an unreasonably large file to read into memory
        if (file_mb > filesize_max_)
        {
            std::ostringstream s;
            s << "CSV Plugin: csv file is greater than " << filesize_max_ << "MB "
              << " - you should use a more efficient data format like sqlite, postgis or a shapefile "
              << " to render this data (set 'filesize_max=0' to disable this restriction if you have lots of memory)";
            throw mapnik::datasource_exception(s.str());
        }
    }

    // set back to start
    stream.seekg(0, std::ios::beg);

    // autodetect newlines
    char newline = '\n';
    bool has_newline = false;
    for (unsigned lidx = 0; lidx < file_length_ && lidx < 4000; lidx++)
    {
        char c = static_cast<char>(stream.get());
        if (c == '\r')
        {
            newline = '\r';
            has_newline = true;
            break;
        }
        if (c == '\n')
        {
            has_newline = true;
            break;
        }
    }

    // set back to start
    stream.seekg(0, std::ios::beg);

    // get first line
    std::string csv_line;
    std::getline(stream,csv_line,newline);

    // if user has not passed a separator manually
    // then attempt to detect by reading first line
    std::string sep = boost::trim_copy(separator);
    if (sep.empty())
    {
        // default to ','
        sep = ",";
        int num_commas = std::count(csv_line.begin(), csv_line.end(), ',');
        // detect tabs
        int num_tabs = std::count(csv_line.begin(), csv_line.end(), '\t');
        if (num_tabs > 0)
        {
            if (num_tabs > num_commas)
            {
                sep = "\t";

                MAPNIK_LOG_DEBUG(csv) << "csv_datasource: auto detected tab separator";
            }
        }
        else // pipes
        {
            int num_pipes = std::count(csv_line.begin(), csv_line.end(), '|');
            if (num_pipes > num_commas)
            {
                sep = "|";

                MAPNIK_LOG_DEBUG(csv) << "csv_datasource: auto detected '|' separator";
            }
            else // semicolons
            {
                int num_semicolons = std::count(csv_line.begin(), csv_line.end(), ';');
                if (num_semicolons > num_commas)
                {
                    sep = ";";

                    MAPNIK_LOG_DEBUG(csv) << "csv_datasource: auto detected ';' separator";
                }
            }
        }
    }

    // set back to start
    stream.seekg(0, std::ios::beg);

    typedef boost::escaped_list_separator<char> escape_type;

    std::string esc = boost::trim_copy(escape);
    if (esc.empty()) esc = "\\";

    std::string quo = boost::trim_copy(quote);
    if (quo.empty()) quo = "\"";

    MAPNIK_LOG_DEBUG(csv) << "csv_datasource: csv grammar: sep: '" << sep
                          << "' quo: '" << quo << "' esc: '" << esc << "'";

    boost::escaped_list_separator<char> grammer;
    try
    {
        //  grammer = boost::escaped_list_separator<char>('\\', ',', '\"');
        grammer = boost::escaped_list_separator<char>(esc, sep, quo);
    }
    catch(std::exception const& ex)
    {
        std::ostringstream s;
        s << "CSV Plugin: " << ex.what();
        throw mapnik::datasource_exception(s.str());
    }

    typedef boost::tokenizer< escape_type > Tokenizer;

    int line_number(1);
    bool has_wkt_field = false;
    bool has_json_field = false;
    bool has_lat_field = false;
    bool has_lon_field = false;
    unsigned wkt_idx(0);
    unsigned json_idx(0);
    unsigned lat_idx(0);
    unsigned lon_idx(0);

    if (!manual_headers_.empty())
    {
        Tokenizer tok(manual_headers_, grammer);
        Tokenizer::iterator beg = tok.begin();
        unsigned idx(0);
        for (; beg != tok.end(); ++beg)
        {
            std::string val = boost::trim_copy(*beg);
            std::string lower_val = boost::algorithm::to_lower_copy(val);
            if (lower_val == "wkt"
                || (lower_val.find("geom") != std::string::npos))
            {
                wkt_idx = idx;
                has_wkt_field = true;
            }
            if (lower_val == "geojson")
            {
                json_idx = idx;
                has_json_field = true;
            }
            if (lower_val == "x"
                || lower_val == "lon"
                || lower_val == "lng"
                || lower_val == "long"
                || (lower_val.find("longitude") != std::string::npos))
            {
                lon_idx = idx;
                has_lon_field = true;
            }
            if (lower_val == "y"
                || lower_val == "lat"
                || (lower_val.find("latitude") != std::string::npos))
            {
                lat_idx = idx;
                has_lat_field = true;
            }
            ++idx;
            headers_.push_back(val);
        }
    }
    else // parse first line as headers
    {
        while (std::getline(stream,csv_line,newline))
        {
            try
            {
                Tokenizer tok(csv_line, grammer);
                Tokenizer::iterator beg = tok.begin();
                std::string val;
                if (beg != tok.end())
                    val = boost::trim_copy(*beg);

                // skip blank lines
                if (val.empty())
                {
                    // do nothing
                    ++line_number;
                }
                else
                {
                    int idx = -1;
                    for (; beg != tok.end(); ++beg)
                    {
                        ++idx;
                        val = boost::trim_copy(*beg);
                        if (val.empty())
                        {
                            if (strict_)
                            {
                                std::ostringstream s;
                                s << "CSV Plugin: expected a column header at line "
                                  << line_number << ", column " << idx
                                  << " - ensure this row contains valid header fields: '"
                                  << csv_line << "'\n";
                                throw mapnik::datasource_exception(s.str());
                            }
                            else
                            {
                                // create a placeholder for the empty header
                                std::ostringstream s;
                                s << "_" << idx;
                                headers_.push_back(s.str());
                            }
                        }
                        else
                        {
                            std::string lower_val = boost::algorithm::to_lower_copy(val);
                            if (lower_val == "wkt"
                                || (lower_val.find("geom") != std::string::npos))
                            {
                                wkt_idx = idx;
                                has_wkt_field = true;
                            }
                            if (lower_val == "geojson")
                            {
                                json_idx = idx;
                                has_json_field = true;
                            }
                            if (lower_val == "x"
                                || lower_val == "lon"
                                || lower_val == "lng"
                                || lower_val == "long"
                                || (lower_val.find("longitude") != std::string::npos))
                            {
                                lon_idx = idx;
                                has_lon_field = true;
                            }
                            if (lower_val == "y"
                                || lower_val == "lat"
                                || (lower_val.find("latitude") != std::string::npos))
                            {
                                lat_idx = idx;
                                has_lat_field = true;
                            }
                            headers_.push_back(val);
                        }
                    }
                    ++line_number;
                    break;
                }
            }
            catch(const std::exception & ex)
            {
                std::ostringstream s;
                s << "CSV Plugin: error parsing headers: " << ex.what();
                throw mapnik::datasource_exception(s.str());
            }
        }
    }

    if (!has_wkt_field && !has_json_field && (!has_lon_field || !has_lat_field) )
    {
        std::ostringstream s;
        s << "CSV Plugin: could not detect column headers with the name of wkt, geojson, x/y, or latitude/longitude - this is required for reading geometry data";
        throw mapnik::datasource_exception(s.str());
    }

    int feature_count(0);
    bool extent_initialized = false;
    std::size_t num_headers = headers_.size();

    for (std::size_t i = 0; i < headers_.size(); ++i)
    {
        ctx_->push(headers_[i]);
    }

    mapnik::transcoder tr(desc_.get_encoding());
    mapnik::wkt_parser parse_wkt;
    mapnik::json::geometry_parser<std::string::const_iterator> parse_json;

    // handle rare case of a single line of data and user-provided headers
    // where a lack of a newline will mean that std::getline returns false
    bool is_first_row = false;
    if (!has_newline)
    {
        stream >> csv_line;
        if (!csv_line.empty())
        {
            is_first_row = true;
        }
    }
    while (std::getline(stream,csv_line,newline) || is_first_row)
    {
        is_first_row = false;
        if ((row_limit_ > 0) && (line_number > row_limit_))
        {
            MAPNIK_LOG_DEBUG(csv) << "csv_datasource: row limit hit, exiting at feature: " << feature_count;
            break;
        }

        // skip blank lines
        unsigned line_length = csv_line.length();
        if (line_length <= 10)
        {
            std::string trimmed = csv_line;
            boost::trim_if(trimmed,boost::algorithm::is_any_of("\",'\r\n "));
            if (trimmed.empty())
            {
                ++line_number;
                MAPNIK_LOG_DEBUG(csv) << "csv_datasource: empty row encountered at line: " << line_number;
                continue;
            }
        }

        try
        {
            // special handling for varieties of quoting that we will enounter with json
            // TODO - test with custom "quo" option
            if (has_json_field && (quo == "\"") && (std::count(csv_line.begin(), csv_line.end(), '"') >= 6))
            {
                csv_utils::fix_json_quoting(csv_line);
            }
            
            Tokenizer tok(csv_line, grammer);
            Tokenizer::iterator beg = tok.begin();

            unsigned num_fields = std::distance(beg,tok.end());
            if (num_fields > num_headers)
            {
                std::ostringstream s;
                s << "CSV Plugin: # of columns("
                << num_fields << ") > # of headers("
                << num_headers << ") parsed for row " << line_number << "\n";
                throw mapnik::datasource_exception(s.str());
            }
            else if (num_fields < num_headers)
            {
                std::ostringstream s;
                s << "CSV Plugin: # of headers("
                << num_headers << ") > # of columns("
                << num_fields << ") parsed for row " << line_number << "\n";
                if (strict_)
                {
                    throw mapnik::datasource_exception(s.str());
                }
                else
                {
                    MAPNIK_LOG_WARN(csv) << s.str();
                }
            }

            // NOTE: we use ++feature_count here because feature id's should start at 1;
            mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,++feature_count));
            double x(0);
            double y(0);
            bool parsed_x = false;
            bool parsed_y = false;
            bool parsed_wkt = false;
            bool parsed_json = false;
            std::vector<std::string> collected;
            for (unsigned i = 0; i < num_headers; ++i)
            {
                std::string fld_name(headers_.at(i));
                collected.push_back(fld_name);
                std::string value;
                if (beg == tok.end()) // there are more headers than column values for this row
                {
                    // add an empty string here to represent a missing value
                    // not using null type here since nulls are not a csv thing
                    feature->put(fld_name,tr.transcode(value.c_str()));
                    if (feature_count == 1)
                    {
                        desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::String));
                    }
                    // continue here instead of break so that all missing values are
                    // encoded consistenly as empty strings
                    continue;
                }
                else
                {
                    value = boost::trim_copy(*beg);
                    ++beg;
                }

                int value_length = value.length();

                // parse wkt
                if (has_wkt_field)
                {
                    if (i == wkt_idx)
                    {
                        // skip empty geoms
                        if (value.empty())
                        {
                            break;
                        }

                        if (parse_wkt.parse(value, feature->paths()))
                        {
                            parsed_wkt = true;
                        }
                        else
                        {
                            std::ostringstream s;
                            s << "CSV Plugin: expected well known text geometry: could not parse row "
                              << line_number
                              << ",column "
                              << i << " - found: '"
                              << value << "'";
                            if (strict_)
                            {
                                throw mapnik::datasource_exception(s.str());
                            }
                            else
                            {
                                MAPNIK_LOG_ERROR(csv) << s.str();
                            }
                        }
                    }
                }
                // TODO - support both wkt/geojson columns
                // at once to create multi-geoms?
                // parse as geojson
                else if (has_json_field)
                {
                    if (i == json_idx)
                    {
                        // skip empty geoms
                        if (value.empty())
                        {
                            break;
                        }
                        if (parse_json.parse(value.begin(),value.end(), feature->paths()))
                        {
                            parsed_json = true;
                        }
                        else
                        {
                            std::ostringstream s;
                            s << "CSV Plugin: expected geojson geometry: could not parse row "
                              << line_number
                              << ",column "
                              << i << " - found: '"
                              << value << "'";
                            if (strict_)
                            {
                                throw mapnik::datasource_exception(s.str());
                            }
                            else
                            {
                                MAPNIK_LOG_ERROR(csv) << s.str();
                            }
                        }
                    }                
                }
                else
                {
                    // longitude
                    if (i == lon_idx)
                    {
                        // skip empty geoms
                        if (value.empty())
                        {
                            break;
                        }

                        if (mapnik::util::string2double(value,x))
                        {
                            parsed_x = true;
                        }
                        else
                        {
                            std::ostringstream s;
                            s << "CSV Plugin: expected a float value for longitude: could not parse row "
                              << line_number
                              << ", column "
                              << i << " - found: '"
                              << value << "'";
                            if (strict_)
                            {
                                throw mapnik::datasource_exception(s.str());
                            }
                            else
                            {
                                MAPNIK_LOG_ERROR(csv) << s.str();
                            }
                        }
                    }
                    // latitude
                    else if (i == lat_idx)
                    {
                        // skip empty geoms
                        if (value.empty())
                        {
                            break;
                        }

                        if (mapnik::util::string2double(value,y))
                        {
                            parsed_y = true;
                        }
                        else
                        {
                            std::ostringstream s;
                            s << "CSV Plugin: expected a float value for latitude: could not parse row "
                              << line_number
                              << ", column "
                              << i << " - found: '"
                              << value << "'";
                            if (strict_)
                            {
                                throw mapnik::datasource_exception(s.str());
                            }
                            else
                            {
                                MAPNIK_LOG_ERROR(csv) << s.str();
                            }
                        }
                    }
                }

                // now, add attributes, skipping any WKT or JSON fiels
                if ((has_wkt_field) && (i == wkt_idx)) continue;
                if ((has_json_field) && (i == json_idx)) continue;
                /* First we detect likely strings, then try parsing likely numbers,
                   finally falling back to string type
                   * We intentionally do not try to detect boolean or null types
                   since they are not common in csv
                   * Likely strings are either empty values, very long values
                   or value with leading zeros like 001 (which are not safe
                   to assume are numbers)
                */

                bool has_dot = value.find(".") != std::string::npos;
                if (value.empty() ||
                    (value_length > 20) ||
                    (value_length > 1 && !has_dot && value[0] == '0'))
                {
                    feature->put(fld_name,tr.transcode(value.c_str()));
                    if (feature_count == 1)
                    {
                        desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::String));
                    }
                }
                else if ((value[0] >= '0' && value[0] <= '9') || value[0] == '-')
                {
                    double float_val = 0.0;
                    std::string::const_iterator str_beg = value.begin();
                    std::string::const_iterator str_end = value.end();
                    bool r = qi::phrase_parse(str_beg,str_end,qi::double_,ascii::space,float_val);
                    if (r && (str_beg == str_end))
                    {
                        if (has_dot)
                        {
                            feature->put(fld_name,float_val);
                            if (feature_count == 1)
                            {
                                desc_.add_descriptor(
                                    mapnik::attribute_descriptor(
                                        fld_name,mapnik::Double));
                            }
                        }
                        else
                        {
                            feature->put(fld_name,static_cast<int>(float_val));
                            if (feature_count == 1)
                            {
                                desc_.add_descriptor(
                                    mapnik::attribute_descriptor(
                                        fld_name,mapnik::Integer));
                            }
                        }
                    }
                    else
                    {
                        // fallback to normal string
                        feature->put(fld_name,tr.transcode(value.c_str()));
                        if (feature_count == 1)
                        {
                            desc_.add_descriptor(
                                mapnik::attribute_descriptor(
                                    fld_name,mapnik::String));
                        }
                    }
                }
                else
                {
                    // fallback to normal string
                    feature->put(fld_name,tr.transcode(value.c_str()));
                    if (feature_count == 1)
                    {
                        desc_.add_descriptor(
                            mapnik::attribute_descriptor(
                                fld_name,mapnik::String));
                    }
                }
            }

            bool null_geom = true;
            if (has_wkt_field || has_json_field)
            {
                if (parsed_wkt || parsed_json)
                {
                    if (!extent_initialized)
                    {
                        extent_initialized = true;
                        extent_ = feature->envelope();
                    }
                    else
                    {
                        extent_.expand_to_include(feature->envelope());
                    }
                    features_.push_back(feature);
                    null_geom = false;
                }
                else
                {
                    std::ostringstream s;
                    s << "CSV Plugin: could not read WKT or GeoJSON geometry "
                      << "for line " << line_number << " - found " <<  headers_.size()
                      << " with values like: " << csv_line << "\n";
                    if (strict_)
                    {
                        throw mapnik::datasource_exception(s.str());
                    }
                    else
                    {
                        MAPNIK_LOG_ERROR(csv) << s.str();
                        continue;
                    }
                }
            }
            else if (has_lat_field || has_lon_field)
            {
                if (parsed_x && parsed_y)
                {
                    mapnik::geometry_type * pt = new mapnik::geometry_type(mapnik::Point);
                    pt->move_to(x,y);
                    feature->add_geometry(pt);
                    features_.push_back(feature);
                    null_geom = false;
                    if (!extent_initialized)
                    {
                        extent_initialized = true;
                        extent_ = feature->envelope();
                    }
                    else
                    {
                        extent_.expand_to_include(feature->envelope());
                    }
                }
                else if (parsed_x || parsed_y)
                {
                    std::ostringstream s;
                    s << "CSV Plugin: does your csv have valid headers?\n";
                    if (!parsed_x)
                    {
                          s << "Could not detect or parse any rows named 'x' or 'longitude' "
                          << "for line " << line_number << " but found " <<  headers_.size()
                          << " with values like: " << csv_line << "\n"
                          << "for: " << boost::algorithm::join(collected, ",") << "\n";
                    }
                    if (!parsed_y)
                    {
                          s << "Could not detect or parse any rows named 'y' or 'latitude' "
                          << "for line " << line_number << " but found " <<  headers_.size()
                          << " with values like: " << csv_line << "\n"
                          << "for: " << boost::algorithm::join(collected, ",") << "\n";
                    }
                    if (strict_)
                    {
                        throw mapnik::datasource_exception(s.str());
                    }
                    else
                    {
                        MAPNIK_LOG_ERROR(csv) << s.str();
                        continue;
                    }
                }
            }

            if (null_geom)
            {
                std::ostringstream s;
                s << "CSV Plugin: could not detect and parse valid lat/lon fields or wkt/json geometry for line "
                  << line_number;
                if (strict_)
                {
                    throw mapnik::datasource_exception(s.str());
                }
                else
                {
                    MAPNIK_LOG_ERROR(csv) << s.str();
                    // with no geometry we will never
                    // add this feature so drop the count
                    feature_count--;
                    continue;
                }
            }

            ++line_number;
        }
        catch(mapnik::datasource_exception const& ex )
        {
            if (strict_)
            {
                throw mapnik::datasource_exception(ex.what());
            }
            else
            {
                MAPNIK_LOG_ERROR(csv) << ex.what();
            }
        }
        catch(std::exception const& ex)
        {
            std::ostringstream s;
            s << "CSV Plugin: unexpected error parsing line: " << line_number
              << " - found " << headers_.size() << " with values like: " << csv_line << "\n"
              << " and got error like: " << ex.what();
            if (strict_)
            {
                throw mapnik::datasource_exception(s.str());
            }
            else
            {
                MAPNIK_LOG_ERROR(csv) << s.str();
            }
        }
    }
    if (!feature_count > 0)
    {
        MAPNIK_LOG_ERROR(csv) << "CSV Plugin: could not parse any lines of data";
    }
}

const char * csv_datasource::name()
{
    return "csv";
}

datasource::datasource_t csv_datasource::type() const
{
    return datasource::Vector;
}

mapnik::box2d<double> csv_datasource::envelope() const
{
    if (!is_bound_) bind();

    return extent_;
}

boost::optional<mapnik::datasource::geometry_t> csv_datasource::get_geometry_type() const
{
    if (! is_bound_) bind();
    boost::optional<mapnik::datasource::geometry_t> result;
    int multi_type = 0;
    unsigned num_features = features_.size();
    for (unsigned i = 0; i < num_features && i < 5; ++i)
    {
        mapnik::util::to_ds_type(features_[i]->paths(),result);
        if (result)
        {
            int type = static_cast<int>(*result);
            if (multi_type > 0 && multi_type != type)
            {
                result.reset(mapnik::datasource::Collection);
                return result;
            }
            multi_type = type;
        }
    }
    return result;
}

mapnik::layer_descriptor csv_datasource::get_descriptor() const
{
    if (!is_bound_) bind();

    return desc_;
}

mapnik::featureset_ptr csv_datasource::features(mapnik::query const& q) const
{
    if (!is_bound_) bind();

    const std::set<std::string>& attribute_names = q.property_names();
    std::set<std::string>::const_iterator pos = attribute_names.begin();
    while (pos != attribute_names.end())
    {
        bool found_name = false;
        for (std::size_t i = 0; i < headers_.size(); ++i)
        {
            if (headers_[i] == *pos)
            {
                found_name = true;
                break;
            }
        }

        if (! found_name)
        {
            std::ostringstream s;

            s << "CSV Plugin: no attribute '" << *pos << "'. Valid attributes are: "
              << boost::algorithm::join(headers_, ",") << ".";

            throw mapnik::datasource_exception(s.str());
        }
        ++pos;
    }

    return boost::make_shared<mapnik::memory_featureset>(q.get_bbox(),features_);
}

mapnik::featureset_ptr csv_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
    if (!is_bound_) bind();

    throw mapnik::datasource_exception("CSV Plugin: features_at_point is not supported yet");
}
