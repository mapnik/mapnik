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

#include "csv_utils.hpp"
#include "csv_datasource.hpp"
#include "csv_featureset.hpp"
#include "csv_inline_featureset.hpp"
// boost
#include <boost/algorithm/string.hpp>
// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/feature_factory.hpp>
#include <mapnik/memory_featureset.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/util/geometry_to_ds_type.hpp>
#include <mapnik/value_types.hpp>
// stl
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(csv_datasource)


namespace {

using cvs_value = mapnik::util::variant<std::string, mapnik::value_integer, mapnik::value_double, mapnik::value_bool>;

}

csv_datasource::csv_datasource(parameters const& params)
: datasource(params),
    desc_(csv_datasource::name(), *params.get<std::string>("encoding", "utf-8")),
    extent_(),
    filename_(),
    row_limit_(*params.get<mapnik::value_integer>("row_limit", 0)),
    inline_string_(),
    escape_(*params.get<std::string>("escape", "")),
    separator_(*params.get<std::string>("separator", "")),
    quote_(*params.get<std::string>("quote", "")),
    headers_(),
    manual_headers_(mapnik::util::trim_copy(*params.get<std::string>("headers", ""))),
    strict_(*params.get<mapnik::boolean_type>("strict", false)),
    ctx_(std::make_shared<mapnik::context_type>()),
    extent_initialized_(false),
    tree_(nullptr),
    locator_()
{
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

csv_datasource::~csv_datasource() {}

template <typename T>
void csv_datasource::parse_csv(T & stream,
                               std::string const& escape,
                               std::string const& separator,
                               std::string const& quote)
{
    auto file_length = detail::file_length(stream);
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
    separator_ = sep;

    // set back to start
    stream.seekg(0, std::ios::beg);

    std::string esc = mapnik::util::trim_copy(escape);
    if (esc.empty()) esc = "\\";

    std::string quo = mapnik::util::trim_copy(quote);
    if (quo.empty()) quo = "\"";

    MAPNIK_LOG_DEBUG(csv) << "csv_datasource: csv grammar: sep: '" << sep
                          << "' quo: '" << quo << "' esc: '" << esc << "'";

    int line_number = 1;
    if (!manual_headers_.empty())
    {
        std::size_t index = 0;
        auto headers = csv_utils::parse_line(manual_headers_, sep);
        for (auto const& header : headers)
        {
            std::string val = mapnik::util::trim_copy(header);
            detail::locate_geometry_column(val, index++, locator_);
            headers_.push_back(val);
        }
    }
    else // parse first line as headers
    {
        while (std::getline(stream,csv_line,stream.widen(newline)))
        {
            try
            {
                auto headers = csv_utils::parse_line(csv_line, sep);
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
                            detail::locate_geometry_column(val, index, locator_);
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

    if (locator_.type == detail::geometry_column_locator::UNKNOWN)
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

    std::vector<item_type> boxes;
    auto pos = stream.tellg();
    while (std::getline(stream, csv_line, stream.widen(newline)) || is_first_row)
    {
        if ((row_limit_ > 0) && (line_number++ > row_limit_))
        {
            MAPNIK_LOG_DEBUG(csv) << "csv_datasource: row limit hit, exiting at feature: " << feature_count;
            break;
        }
        auto record_offset = pos;
        auto record_size = csv_line.length();
        pos = stream.tellg();
        is_first_row = false;
        // skip blank lines
        unsigned line_length = csv_line.length();
        if (line_length <= 10)
        {
            std::string trimmed = csv_line;
            boost::trim_if(trimmed,boost::algorithm::is_any_of("\",'\r\n "));
            if (trimmed.empty())
            {
                MAPNIK_LOG_DEBUG(csv) << "csv_datasource: empty row encountered at line: " << line_number;
                continue;
            }
        }

        try
        {
            auto values = csv_utils::parse_line(csv_line, sep);
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

            auto geom = detail::extract_geometry(values, locator_);
            if (!geom.is<mapnik::geometry::geometry_empty>())
            {
                auto box = mapnik::geometry::envelope(geom);
                boxes.emplace_back(std::move(box), make_pair(record_offset, record_size));
                if (!extent_initialized_)
                {
                    if (!extent_started)
                    {
                        extent_started = true;
                        extent_ = mapnik::geometry::envelope(geom);
                    }
                    else
                    {
                        extent_.expand_to_include(mapnik::geometry::envelope(geom));
                    }
                }
                if (++feature_count != 1) continue;
                auto beg = values.begin();
                auto end = values.end();
                for (std::size_t i = 0; i < num_headers; ++i)
                {
                    std::string const& header = headers_.at(i);
                    if (beg == end) // there are more headers than column values for this row
                    {
                        // add an empty string here to represent a missing value
                        // not using null type here since nulls are not a csv thing
                        if (feature_count == 1)
                        {
                            desc_.add_descriptor(mapnik::attribute_descriptor(header, mapnik::String));
                        }
                        // continue here instead of break so that all missing values are
                        // encoded consistenly as empty strings
                        continue;
                    }
                    std::string value = mapnik::util::trim_copy(*beg++);
                    int value_length = value.length();
                    if (locator_.index == i && (locator_.type == detail::geometry_column_locator::WKT
                                                || locator_.type == detail::geometry_column_locator::GEOJSON)) continue;

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
                    if (value.empty() || (value_length > 20) || (value_length > 1 && !has_dot && value[0] == '0'))
                    {
                        matched = true;
                        desc_.add_descriptor(mapnik::attribute_descriptor(header, mapnik::String));
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
                                desc_.add_descriptor(mapnik::attribute_descriptor(header,mapnik::Double));
                            }
                        }
                        else
                        {
                            mapnik::value_integer int_val = 0;
                            if (mapnik::util::string2int(value,int_val))
                            {
                                matched = true;
                                desc_.add_descriptor(mapnik::attribute_descriptor(header,mapnik::Integer));
                            }
                        }
                    }
                    if (!matched)
                    {
                        // NOTE: we don't use mapnik::util::string2bool
                        // here because we don't want to treat 'on' and 'off'
                        // as booleans, only 'true' and 'false'
                        if (csv_utils::ignore_case_equal(value, "true") || csv_utils::ignore_case_equal(value, "false"))
                        {
                            desc_.add_descriptor(mapnik::attribute_descriptor(header, mapnik::Boolean));
                        }
                        else // fallback to normal string
                        {
                            desc_.add_descriptor(mapnik::attribute_descriptor(header, mapnik::String));
                        }
                    }
                }
            }
            else
            {
                std::ostringstream s;
                s << "CSV Plugin: expected geometry column: could not parse row "
                  << line_number << " "
                  << values[locator_.index] << "'";
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
        catch (mapnik::datasource_exception const& ex )
        {
            if (strict_) throw ex;
            else
            {
                MAPNIK_LOG_ERROR(csv) << ex.what();
            }
        }
        catch (std::exception const& ex)
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
    // bulk insert initialise r-tree
    tree_ = std::make_unique<spatial_index_type>(boxes);
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
    auto itr = tree_->qbegin(boost::geometry::index::intersects(extent_));
    auto end = tree_->qend();
    mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
    for (std::size_t count = 0; itr !=end &&  count < 5; ++itr, ++count)
    {
        csv_datasource::item_type const& item = *itr;
        std::size_t file_offset = item.second.first;
        std::size_t size = item.second.second;

        std::string str;
        if (inline_string_.empty())
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
            in.seekg(file_offset);
            std::vector<char> record;
            record.resize(size);
            in.read(record.data(), size);
            str = std::string(record.begin(), record.end());
        }
        else
        {
            str = inline_string_.substr(file_offset, size);
        }

        try
        {
            auto values = csv_utils::parse_line(str, separator_);
            auto geom = detail::extract_geometry(values, locator_);
            result = mapnik::util::to_ds_type(geom);
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
        catch (std::exception const& ex)
        {
            if (strict_) throw ex;
            else MAPNIK_LOG_ERROR(csv) << ex.what();
        }
    }
    return result;
}

mapnik::featureset_ptr csv_datasource::features(mapnik::query const& q) const
{

    for (auto const& name : q.property_names())
    {
        bool found_name = false;
        for (auto const& header : headers_)
        {
            if (header == name)
            {
                found_name = true;
                break;
            }
        }
        if (!found_name)
        {
            std::ostringstream s;
            s << "CSV Plugin: no attribute '" << name << "'. Valid attributes are: "
              << boost::algorithm::join(headers_, ",") << ".";
            throw mapnik::datasource_exception(s.str());
        }
    }

    mapnik::box2d<double> const& box = q.get_bbox();
    if (extent_.intersects(box))
    {
        csv_featureset::array_type index_array;
        if (tree_)
        {
            tree_->query(boost::geometry::index::intersects(box),std::back_inserter(index_array));
            std::sort(index_array.begin(),index_array.end(),
                      [] (item_type const& item0, item_type const& item1)
                      {
                          return item0.second.first < item1.second.first;
                      });
            if (inline_string_.empty())
            {
                return std::make_shared<csv_featureset>(filename_, locator_, separator_, headers_, ctx_, std::move(index_array));
            }
            else
            {
                return std::make_shared<csv_inline_featureset>(inline_string_, locator_, separator_, headers_, ctx_, std::move(index_array));
            }
        }
    }
    return mapnik::featureset_ptr();
}

mapnik::featureset_ptr csv_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
    mapnik::box2d<double> query_bbox(pt, pt);
    query_bbox.pad(tol);
    mapnik::query q(query_bbox);
    std::vector<mapnik::attribute_descriptor> const& desc = desc_.get_descriptors();
    for (auto const& item : desc)
    {
        q.add_property_name(item.get_name());
    }
    return features(q);
}
