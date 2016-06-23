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

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_correct.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/json/geometry_parser.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/datasource.hpp>
// csv grammar
#include <mapnik/csv/csv_grammar_impl.hpp>
//
#include "csv_getline.hpp"
#include "csv_utils.hpp"

#include <fstream>
#include <string>
#include <cstdio>
#include <algorithm>

namespace csv_utils {
namespace detail {

std::size_t file_length(std::istream & stream)
{
    stream.seekg(0, std::ios::end);
    return stream.tellg();
}

std::tuple<char, bool, char, char> autodetect_csv_flavour(std::istream & stream, std::size_t file_length)
{
    // autodetect newlines/quotes/separators
    char newline = '\n'; // default
    bool has_newline = false;
    bool has_single_quote = false;
    char quote = '"'; // default
    char separator = ','; // default
    // local counters
    int num_commas = 0;
    int num_tabs = 0;
    int num_pipes = 0;
    int num_semicolons = 0;

    static std::size_t const max_size = 4000;
    std::size_t size = std::min(file_length, max_size);
    std::vector<char> buffer;
    buffer.resize(size);
    stream.read(buffer.data(), size);
    for (auto c : buffer)
    {
        switch (c)
        {
        case '\r':
            newline = '\r';
            has_newline = true;
            break;
        case '\n':
            has_newline = true;
            break;
        case '\'':
            if (!has_single_quote)
            {
                quote = c;
                has_single_quote = true;
            }
            break;
        case ',':
            if (!has_newline) ++num_commas;
            break;
        case '\t':
            if (!has_newline) ++num_tabs;
            break;
        case '|':
            if (!has_newline) ++num_pipes;
            break;
        case ';':
           if (!has_newline) ++num_semicolons;
            break;
        }
    }
    // detect separator
    if (num_tabs > 0 && num_tabs > num_commas)
    {
        separator = '\t';
        MAPNIK_LOG_DEBUG(csv) << "csv_datasource: auto detected tab separator";
    }
    else // pipes/semicolons
    {
        if (num_pipes > num_commas)
        {
            separator = '|';
            MAPNIK_LOG_DEBUG(csv) << "csv_datasource: auto detected '|' separator";
        }
        else  if (num_semicolons > num_commas)
        {
            separator = ';';
            MAPNIK_LOG_DEBUG(csv) << "csv_datasource: auto detected ';' separator";
        }
    }

    if (has_newline && has_single_quote)
    {
        std::istringstream ss(std::string(buffer.begin(), buffer.end()));
        std::size_t num_columns = 0;
        for (std::string line; csv_utils::getline_csv(ss, line, newline, quote); )
        {
            if (size < file_length && ss.eof())
            {
                // we can't be sure that last line
                // is not truncated so skip it
                break;
            }
            if (line.size() == 0 || (line.size() == 1 && line[0] == char(0xa))) continue; // empty lines are not interesting
            auto num_quotes = std::count(line.begin(), line.end(), quote);
            if (num_quotes % 2 != 0)
            {
                quote = '"';
                break;
            }
            auto columns = csv_utils::parse_line(line, separator, quote);
            if (num_columns > 0 && num_columns != columns.size())
            {
                quote = '"';
                break;
            }
            num_columns = columns.size();
        }
    }
    return std::make_tuple(newline, has_newline, separator, quote);
}

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

bool valid(geometry_column_locator const& locator, std::size_t max_size)
{
    if (locator.type == geometry_column_locator::UNKNOWN) return false;
    if (locator.index >= max_size) return false;
    if (locator.type == geometry_column_locator::LON_LAT && locator.index2 >= max_size) return false;
    return true;
}

} // namespace detail

static const mapnik::csv_line_grammar<char const*> line_g;
static const mapnik::csv_white_space_skipper skipper{};

mapnik::csv_line parse_line(char const* start, char const* end, char separator, char quote, std::size_t num_columns)
{
    mapnik::csv_line values;
    if (num_columns > 0) values.reserve(num_columns);
    if (!boost::spirit::qi::phrase_parse(start, end, (line_g)(separator, quote), skipper, values))
    {
        throw mapnik::datasource_exception("Failed to parse CSV line:\n" + std::string(start, end));
    }
    return values;
}

mapnik::csv_line parse_line(std::string const& line_str, char separator, char quote)
{
    auto start = line_str.c_str();
    auto end   = start + line_str.length();
    return parse_line(start, end, separator, quote, 0);
}


bool is_likely_number(std::string const& value)
{
    return (std::strspn( value.c_str(), "e-.+0123456789" ) == value.size());
}

struct ignore_case_equal_pred
{
    bool operator () (unsigned char a, unsigned char b) const
    {
        return std::tolower(a) == std::tolower(b);
    }
};

bool ignore_case_equal(std::string const& s0, std::string const& s1)
{
    return std::equal(s0.begin(), s0.end(),
                      s1.begin(), ignore_case_equal_pred());
}

void csv_file_parser::add_feature(mapnik::value_integer, mapnik::csv_line const & )
{
    // no-op by default
}

template <typename T>
void csv_file_parser::parse_csv_and_boxes(std::istream & csv_file, T & boxes)
{
    using boxes_type = T;
    using box_type = typename boxes_type::value_type::first_type;

    auto file_length = detail::file_length(csv_file);
    // set back to start
    csv_file.seekg(0, std::ios::beg);
    char newline;
    bool has_newline;
    char detected_quote;
    char detected_separator;
    std::tie(newline, has_newline, detected_separator, detected_quote) = detail::autodetect_csv_flavour(csv_file, file_length);
    if (quote_ == 0) quote_ = detected_quote;
    if (separator_ == 0) separator_ = detected_separator;

    // set back to start
    MAPNIK_LOG_DEBUG(csv) << "csv_datasource: separator: '" << separator_
                          << "' quote: '" << quote_ << "'";

    // rewind stream
    csv_file.seekg(0, std::ios::beg);
    //
    std::string csv_line;
    csv_utils::getline_csv(csv_file, csv_line, newline, quote_);
    csv_file.seekg(0, std::ios::beg);
    int line_number = 0;
    if (!manual_headers_.empty())
    {
        std::size_t index = 0;
        auto headers = csv_utils::parse_line(manual_headers_, separator_, quote_);
        headers_.reserve(headers.size());
        for (auto const& header : headers)
        {
            detail::locate_geometry_column(header, index++, locator_);
            headers_.push_back(header);
        }
    }
    else // parse first line as headers
    {
        while (csv_utils::getline_csv(csv_file, csv_line, newline, quote_))
        {
            try
            {
                auto headers = csv_utils::parse_line(csv_line, separator_, quote_);
                // skip blank lines
                if (headers.size() > 0 && headers[0].empty()) ++line_number;
                else
                {
                    std::size_t index = 0;
                    headers_.reserve(headers.size());
                    for (auto & header : headers)
                    {
                        mapnik::util::trim(header);
                        if (header.empty())
                        {
                            if (strict_)
                            {
                                std::ostringstream s;
                                s << "CSV Plugin: expected a column header at line ";
                                s << line_number << ", column " << index;
                                s << " - ensure this row contains valid header fields: '";
                                s << csv_line;
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
                            detail::locate_geometry_column(header, index, locator_);
                            headers_.push_back(header);
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

    std::size_t num_headers = headers_.size();
    if (!detail::valid(locator_, num_headers))
    {
        std::string str("CSV Plugin: could not detect column(s) with the name(s) of wkt, geojson, x/y, or ");
        str += "latitude/longitude in:\n";
        str += csv_line;
        str += "\n - this is required for reading geometry data";
        throw mapnik::datasource_exception(str);
    }

    mapnik::value_integer feature_count = 0;
    auto pos = csv_file.tellg();
    // handle rare case of a single line of data and user-provided headers
    // where a lack of a newline will mean that csv_utils::getline_csv returns false
    bool is_first_row = false;

    if (!has_newline)
    {
        csv_file.setstate(std::ios::failbit);
        pos = 0;
        if (!csv_line.empty())
        {
            is_first_row = true;
        }
    }

    while (is_first_row || csv_utils::getline_csv(csv_file, csv_line, newline, quote_))
    {
        ++line_number;
        if ((row_limit_ > 0) && (line_number > row_limit_))
        {
            MAPNIK_LOG_DEBUG(csv) << "csv_datasource: row limit hit, exiting at feature: " << feature_count;
            break;
        }
        auto record_offset = pos;
        auto record_size = csv_line.length();
        pos = csv_file.tellg();
        is_first_row = false;

        // skip blank lines
        if (record_size <= 10)
        {
            std::string trimmed = csv_line;
            boost::trim_if(trimmed, boost::algorithm::is_any_of("\",'\r\n "));
            if (trimmed.empty())
            {
                MAPNIK_LOG_DEBUG(csv) << "csv_datasource: empty row encountered at line: " << line_number;
                continue;
            }
        }

        try
        {
            auto const* line_start = csv_line.data();
            auto const* line_end = line_start + csv_line.size();
            auto values = csv_utils::parse_line(line_start, line_end, separator_, quote_, num_headers);
            unsigned num_fields = values.size();
            if (num_fields != num_headers)
            {
                std::ostringstream s;
                s << "CSV Plugin: # of columns(" << num_fields << ")";
                if (num_fields > num_headers)
                {
                    s << " > ";
                }
                else
                {
                    s << " < ";
                }
                s << "# of headers(" << num_headers << ") parsed";
                throw mapnik::datasource_exception(s.str());
            }

            auto geom = extract_geometry(values, locator_);
            if (!geom.is<mapnik::geometry::geometry_empty>())
            {
                auto box = mapnik::geometry::envelope(geom);
                if (!extent_initialized_)
                {
                    if (extent_.valid())
                        extent_.expand_to_include(box);
                    else
                        extent_ = box;
                }
                boxes.emplace_back(box_type(box), make_pair(record_offset, record_size));
                add_feature(++feature_count, values);
            }
            else
            {
                std::ostringstream s;
                s << "CSV Plugin: expected geometry column: could not parse row "
                  << line_number << " "
                  << values.at(locator_.index) << "'";
                throw mapnik::datasource_exception(s.str());
            }
        }
        catch (mapnik::datasource_exception const& ex )
        {
            if (strict_) throw ex;
            else
            {
                MAPNIK_LOG_ERROR(csv) << ex.what() << " at line: " << line_number;
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
        // return early if *.index is present
        if (has_disk_index_) return;
    }
}


mapnik::geometry::geometry<double> extract_geometry(std::vector<std::string> const& row, geometry_column_locator const& locator)
{
    mapnik::geometry::geometry<double> geom;
    if (locator.type == geometry_column_locator::WKT)
    {
        auto wkt_value = row.at(locator.index);
        if (mapnik::from_wkt(wkt_value, geom))
        {
            // correct orientations ..
            mapnik::geometry::correct(geom);
        }
        else
        {
            throw mapnik::datasource_exception("Failed to parse WKT: '" + wkt_value + "'");
        }
    }
    else if (locator.type == geometry_column_locator::GEOJSON)
    {

        auto json_value = row.at(locator.index);
        if (!mapnik::json::from_geojson(json_value, geom))
        {
            throw mapnik::datasource_exception("Failed to parse GeoJSON: '" + json_value + "'");
        }
    }
    else if (locator.type == geometry_column_locator::LON_LAT)
    {
        double x, y;
        auto long_value = row.at(locator.index);
        auto lat_value = row.at(locator.index2);
        if (!mapnik::util::string2double(long_value,x))
        {
            throw mapnik::datasource_exception("Failed to parse Longitude: '" + long_value + "'");
        }
        if (!mapnik::util::string2double(lat_value,y))
        {
            throw mapnik::datasource_exception("Failed to parse Latitude: '" + lat_value + "'");
        }
        geom = mapnik::geometry::point<double>(x,y);
    }
    return geom;
}

template void csv_file_parser::parse_csv_and_boxes(std::istream & csv_file, std::vector<std::pair<mapnik::box2d<double>, std::pair<std::size_t, std::size_t>>> & boxes);

template void csv_file_parser::parse_csv_and_boxes(std::istream & csv_file, std::vector<std::pair<mapnik::box2d<float>, std::pair<std::size_t, std::size_t>>> & boxes);

} // namespace csv_utils
