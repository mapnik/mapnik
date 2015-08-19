/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_correct.hpp>
#include <mapnik/geometry_is_empty.hpp>
#include <mapnik/memory_featureset.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/json/geometry_parser.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/util/geometry_to_ds_type.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/csv/csv_grammar.hpp>
// stl
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(csv_datasource)

namespace mapnik {

static const csv_line_grammar<char const*> line_g;

csv_line parse_line(std::string & line_str, std::string const& separator)
{
    csv_line values;
    auto start = line_str.c_str();
    auto end   = start + line_str.length();
    boost::spirit::standard::blank_type blank;
    if (!boost::spirit::qi::phrase_parse(start, end, (line_g)(boost::phoenix::cref(separator)), blank, values))
    {
        throw std::runtime_error("Failed to parse CSV line:\n" + line_str);
    }
    return values;
}
}

csv_datasource::csv_datasource(parameters const& params)
  : datasource(params),
    desc_(csv_datasource::name(), *params.get<std::string>("encoding", "utf-8")),
    extent_(),
    filename_(),
    inline_string_(),
    row_limit_(*params.get<mapnik::value_integer>("row_limit", 0)),
    features_(),
    escape_(*params.get<std::string>("escape", "")),
    separator_(*params.get<std::string>("separator", "")),
    quote_(*params.get<std::string>("quote", "")),
    headers_(),
    manual_headers_(mapnik::util::trim_copy(*params.get<std::string>("headers", ""))),
    strict_(*params.get<mapnik::boolean_type>("strict", false)),
    filesize_max_(*params.get<double>("filesize_max", 20.0)),  // MB
    ctx_(std::make_shared<mapnik::context_type>()),
    extent_initialized_(false)
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

    boost::optional<std::string> ext = params.get<std::string>("extent");
    if (ext && !ext->empty())
    {
        extent_initialized_ = extent_.from_string(*ext);
    }

    boost::optional<std::string> inline_string = params.get<std::string>("inline");
    if (inline_string)
    {
        inline_string_ = *inline_string;
    }
    else
    {
        boost::optional<std::string> file = params.get<std::string>("file");
        if (!file) throw mapnik::datasource_exception("CSV Plugin: missing <file> parameter");

        boost::optional<std::string> base = params.get<std::string>("base");
        if (base)
            filename_ = *base + "/" + *file;
        else
            filename_ = *file;
    }
    if (!inline_string_.empty())
    {
        std::istringstream in(inline_string_);
        parse_csv(in, escape_, separator_, quote_);
    }
    else
    {
#if defined (_WINDOWS)
        std::ifstream in(mapnik::utf8_to_utf16(filename_),std::ios_base::in | std::ios_base::binary);
#else
        std::ifstream in(filename_.c_str(),std::ios_base::in | std::ios_base::binary);
#endif
        if (!in.is_open())
        {
            throw mapnik::datasource_exception("CSV Plugin: could not open: '" + filename_ + "'");
        }
        parse_csv(in, escape_, separator_, quote_);
        in.close();
    }
}


csv_datasource::~csv_datasource() { }

namespace detail {

template <typename T>
std::size_t file_length(T & stream)
{
    stream.seekg(0, std::ios::end);
    return stream.tellg();
}

std::string detect_separator(std::string const& str)
{
    std::string separator = ","; // default
    int num_commas = std::count(str.begin(), str.end(), ',');
    // detect tabs
    int num_tabs = std::count(str.begin(), str.end(), '\t');
    if (num_tabs > 0)
    {
        if (num_tabs > num_commas)
        {
            separator = "\t";

            MAPNIK_LOG_DEBUG(csv) << "csv_datasource: auto detected tab separator";
        }
    }
    else // pipes
    {
        int num_pipes = std::count(str.begin(), str.end(), '|');
        if (num_pipes > num_commas)
        {
            separator = "|";

            MAPNIK_LOG_DEBUG(csv) << "csv_datasource: auto detected '|' separator";
        }
        else // semicolons
        {
            int num_semicolons = std::count(str.begin(), str.end(), ';');
            if (num_semicolons > num_commas)
            {
                separator = ";";
                MAPNIK_LOG_DEBUG(csv) << "csv_datasource: auto detected ';' separator";
            }
        }
    }
    return separator;
}

template <typename T>
std::tuple<char,bool> autodect_newline(T & stream, std::size_t file_length)
{
    // autodetect newlines
    char newline = '\n';
    bool has_newline = false;
    for (std::size_t lidx = 0; lidx < file_length && lidx < 4000; ++lidx)
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
    return std::make_tuple(newline,has_newline);
}


struct geometry_column_locator
{
    geometry_column_locator()
        : type(UNKNOWN), index(-1), index2(-1) {}

    enum { UNKNOWN = 0, WKT, GEOJSON, LON_LAT } type;
    std::size_t index;
    std::size_t index2;
};

void locate_geometry_column(std::string const& header, std::size_t index, geometry_column_locator & locator)
{
    std::string lower_val(header);
    std::transform(lower_val.begin(), lower_val.end(), lower_val.begin(), ::tolower);
    if (lower_val == "wkt" || (lower_val.find("geom") != std::string::npos))
    {
        locator.type = geometry_column_locator::WKT;
        locator.index = index;
    }
    else if (lower_val == "geojson")
    {
        locator.type = geometry_column_locator::GEOJSON;
        locator.index = index;
    }
    else if (lower_val == "x" || lower_val == "lon"
        || lower_val == "lng" || lower_val == "long"
             || (lower_val.find("longitude") != std::string::npos))
    {
        locator.index = index;
        locator.type = geometry_column_locator::LON_LAT;
    }

    else if (lower_val == "y"
             || lower_val == "lat"
             || (lower_val.find("latitude") != std::string::npos))
    {
        locator.index2 = index;
        locator.type = geometry_column_locator::LON_LAT;
    }
}

mapnik::geometry::geometry<double> extract_geometry(std::vector<std::string> const& row, geometry_column_locator const& locator)
{
    mapnik::geometry::geometry<double> geom;
    if (locator.type == geometry_column_locator::WKT)
    {
        if (mapnik::from_wkt(row[locator.index], geom))
        {
            // correct orientations ..
            mapnik::geometry::correct(geom);
        }
        else
        {
            throw std::runtime_error("FIXME WKT");
        }
    }
    else if (locator.type == geometry_column_locator::GEOJSON)
    {

        if (!mapnik::json::from_geojson(row[locator.index], geom))
        {
            throw std::runtime_error("FIXME GEOJSON");
        }
    }
    else if (locator.type == geometry_column_locator::LON_LAT)
    {
        double x, y;
        if (!mapnik::util::string2double(row[locator.index],x))
        {
            throw std::runtime_error("FIXME Lon");
        }
        if (!mapnik::util::string2double(row[locator.index2],y))
        {

            throw std::runtime_error("FIXME Lat");
        }
        geom = mapnik::geometry::point<double>(x,y);
    }
    return geom;
}

} // ns detail

template <typename T>
void csv_datasource::parse_csv(T & stream,
                               std::string const& escape,
                               std::string const& separator,
                               std::string const& quote)
{

    auto file_length = detail::file_length(stream);
    /*
    if (filesize_max_ > 0)
    {
        double file_mb = static_cast<double>(file_length)/1048576;

        // throw if this is an unreasonably large file to read into memory
        if (file_mb > filesize_max_)
        {
            std::ostringstream s;
            s << "CSV Plugin: csv file is greater than ";
            s << filesize_max_ << "MB - you should use a more efficient data format like sqlite,";
            s << "postgis or a shapefile to render this data (set 'filesize_max=0' to disable this restriction if you have lots of memory)";
            throw mapnik::datasource_exception(s.str());
        }
    }
    */

    // set back to start
    stream.seekg(0, std::ios::beg);
    char newline;
    bool has_newline;
    std::tie(newline, has_newline) = detail::autodect_newline(stream, file_length);
    // set back to start
    stream.seekg(0, std::ios::beg);

    // get first line
    std::string csv_line;
    std::getline(stream,csv_line,stream.widen(newline));

    // if user has not passed a separator manually
    // then attempt to detect by reading first line
    std::string sep = mapnik::util::trim_copy(separator);
    if (sep.empty())  sep = detail::detect_separator(csv_line);
    // set back to start
    stream.seekg(0, std::ios::beg);

    std::string esc = mapnik::util::trim_copy(escape);
    if (esc.empty()) esc = "\\";

    std::string quo = mapnik::util::trim_copy(quote);
    if (quo.empty()) quo = "\"";

    MAPNIK_LOG_DEBUG(csv) << "csv_datasource: csv grammar: sep: '" << sep
                          << "' quo: '" << quo << "' esc: '" << esc << "'";

    int line_number = 1;
    detail::geometry_column_locator locator;

    if (!manual_headers_.empty())
    {
        std::size_t index = 0;
        auto headers = mapnik::parse_line(manual_headers_, sep);
        for (auto const& header : headers)
        {
            std::string val = mapnik::util::trim_copy(header);
            detail::locate_geometry_column(val, index++, locator);
            headers_.push_back(val);
        }
    }
    else // parse first line as headers
    {
        while (std::getline(stream,csv_line,stream.widen(newline)))
        {
            try
            {
                auto headers = mapnik::parse_line(csv_line, sep);
                // skip blank lines
                std::string val;
                if (headers.size() > 0 && headers[0].empty()) ++line_number;
                else
                {
                    std::size_t index = 0;
                    for (auto const& header : headers)
                    {
                        val = mapnik::util::trim_copy(header);
                        if (val.empty())
                        {
                            if (strict_)
                            {
                                std::ostringstream s;
                                s << "CSV Plugin: expected a column header at line ";
                                s << line_number << ", column " << index;
                                s << " - ensure this row contains valid header fields: '";
                                s << csv_line << "'\n";
                                throw mapnik::datasource_exception(s.str());
                            }
                            else
                            {
                                // create a placeholder for the empty header
                                std::ostringstream s;
                                s << "_" << index;
                                headers_.push_back(s.str());
                            }
                        }
                        else
                        {
                            detail::locate_geometry_column(val, index, locator);
                            headers_.push_back(val);
                        }
                        ++index;
                    }
                    ++line_number;
                    break;
                }
            }
            catch (std::exception const& ex)
            {
                std::string s("CSV Plugin: error parsing headers: ");
                s += ex.what();
                throw mapnik::datasource_exception(s);
            }
        }
    }

    if (locator.type == detail::geometry_column_locator::UNKNOWN)
    {
        throw mapnik::datasource_exception("CSV Plugin: could not detect column headers with the name of wkt, geojson, x/y, or "
                                           "latitude/longitude - this is required for reading geometry data");
    }

    mapnik::value_integer feature_count = 0;
    bool extent_started = false;

    std::size_t num_headers = headers_.size();
    std::for_each(headers_.begin(), headers_.end(),
                  [ & ](std::string const& header){ ctx_->push(header); });

    mapnik::transcoder tr(desc_.get_encoding());

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
    while (std::getline(stream,csv_line, stream.widen(newline)) || is_first_row)
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
            auto values = mapnik::parse_line(csv_line, sep);
            unsigned num_fields = values.size();
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

            auto beg = values.begin();
            auto end = values.end();


            auto geom = detail::extract_geometry(values, locator);
            if (!geom.is<mapnik::geometry::geometry_empty>())
            {

                mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_, ++feature_count));
                feature->set_geometry(std::move(geom));

                std::vector<std::string> collected;
                for (unsigned i = 0; i < num_headers; ++i)
                {
                    std::string const& fld_name = headers_.at(i);
                    collected.push_back(fld_name);
                    std::string value;
                    if (beg == end) // there are more headers than column values for this row
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
                        value = mapnik::util::trim_copy(*beg++);
                    }
                    int value_length = value.length();

                    // now, add attributes, skipping any WKT or JSON fields
                    if (locator.index == i && (locator.type == detail::geometry_column_locator::WKT
                                               || locator.type == detail::geometry_column_locator::GEOJSON)  ) continue;

                    // First we detect likely strings,
                    // then try parsing likely numbers,
                    // then try converting to bool,
                    // finally falling back to string type.
                    // An empty string or a string of "null" will be parsed
                    // as a string rather than a true null value.
                    // Likely strings are either empty values, very long values
                    // or values with leading zeros like 001 (which are not safe
                    // to assume are numbers)

                    bool matched = false;
                    bool has_dot = value.find(".") != std::string::npos;
                    if (value.empty() ||
                        (value_length > 20) ||
                        (value_length > 1 && !has_dot && value[0] == '0'))
                    {
                        matched = true;
                        feature->put(fld_name,std::move(tr.transcode(value.c_str())));
                        if (feature_count == 1)
                        {
                            desc_.add_descriptor(mapnik::attribute_descriptor(fld_name,mapnik::String));
                        }
                    }
                    else if (csv_utils::is_likely_number(value))
                    {
                        bool has_e = value.find("e") != std::string::npos;
                        if (has_dot || has_e)
                        {
                            double float_val = 0.0;
                            if (mapnik::util::string2double(value,float_val))
                            {
                                matched = true;
                                feature->put(fld_name,float_val);
                                if (feature_count == 1)
                                {
                                    desc_.add_descriptor(
                                        mapnik::attribute_descriptor(
                                            fld_name,mapnik::Double));
                                }
                            }
                        }
                        else
                        {
                            mapnik::value_integer int_val = 0;
                            if (mapnik::util::string2int(value,int_val))
                            {
                                matched = true;
                                feature->put(fld_name,int_val);
                                if (feature_count == 1)
                                {
                                    desc_.add_descriptor(
                                        mapnik::attribute_descriptor(
                                            fld_name,mapnik::Integer));
                                }
                            }
                        }
                    }
                    if (!matched)
                    {
                        // NOTE: we don't use mapnik::util::string2bool
                        // here because we don't want to treat 'on' and 'off'
                        // as booleans, only 'true' and 'false'
                        bool bool_val = false;
                        std::string lower_val = value;
                        std::transform(lower_val.begin(), lower_val.end(), lower_val.begin(), ::tolower);
                        if (lower_val == "true")
                        {
                            matched = true;
                            bool_val = true;
                        }
                        else if (lower_val == "false")
                        {
                            matched = true;
                            bool_val = false;
                        }
                        if (matched)
                        {
                            feature->put(fld_name,bool_val);
                            if (feature_count == 1)
                            {
                                desc_.add_descriptor(
                                    mapnik::attribute_descriptor(
                                        fld_name,mapnik::Boolean));
                            }
                        }
                        else
                        {
                            // fallback to normal string
                            feature->put(fld_name,std::move(tr.transcode(value.c_str())));
                            if (feature_count == 1)
                            {
                                desc_.add_descriptor(
                                    mapnik::attribute_descriptor(
                                        fld_name,mapnik::String));
                            }
                        }
                    }
                }
                bool null_geom = true;
                if (locator.type == detail::geometry_column_locator::WKT
                    || locator.type == detail::geometry_column_locator::GEOJSON
                    || locator.type == detail::geometry_column_locator::LON_LAT)
                {
                    //if (parsed_wkt || parsed_json)
                    //{
                    if (!extent_initialized_)
                    {
                        if (!extent_started)
                        {
                            extent_started = true;
                            extent_ = feature->envelope();
                        }
                        else
                        {
                            extent_.expand_to_include(feature->envelope());
                        }
                    }
                    features_.push_back(feature);
                    null_geom = false;
                }
                else
                {
                    throw "FIXME";
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
            }
            else
            {
                std::ostringstream s;
                s << "CSV Plugin: expected geometry column: could not parse row "
                  << line_number << " "
                  << values[locator.index] << "'";
                if (strict_)
                {
                    throw mapnik::datasource_exception(s.str());
                }
                else
                {
                    MAPNIK_LOG_ERROR(csv) << s.str();
                }
            }


            ++line_number;
        }
        catch (mapnik::datasource_exception const& ex )
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
    if (feature_count < 1)
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
    return extent_;
}

mapnik::layer_descriptor csv_datasource::get_descriptor() const
{
    return desc_;
}

boost::optional<mapnik::datasource_geometry_t> csv_datasource::get_geometry_type() const
{
    boost::optional<mapnik::datasource_geometry_t> result;
    int multi_type = 0;
    unsigned num_features = features_.size();
    for (unsigned i = 0; i < num_features && i < 5; ++i)
    {
        result = mapnik::util::to_ds_type(features_[i]->get_geometry());
        if (result)
        {
            int type = static_cast<int>(*result);
            if (multi_type > 0 && multi_type != type)
            {
                result.reset(mapnik::datasource_geometry_t::Collection);
                return result;
            }
            multi_type = type;
        }
    }
    return result;
}

mapnik::featureset_ptr csv_datasource::features(mapnik::query const& q) const
{
    std::set<std::string> const& attribute_names = q.property_names();
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
    return std::make_shared<mapnik::memory_featureset>(q.get_bbox(),features_);
}

mapnik::featureset_ptr csv_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
    throw mapnik::datasource_exception("CSV Plugin: features_at_point is not supported yet");
}
