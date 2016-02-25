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
#include <mapnik/csv/csv_grammar.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/datasource.hpp>
#include "csv_getline.hpp"
#include "csv_utils.hpp"

#include <fstream>
#include <string>
#include <cstdio>
#include <algorithm>

namespace csv_utils
{

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

}


namespace detail {

std::tuple<char, bool, char, char> autodect_csv_flavour(std::istream & stream, std::size_t file_length)
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
                // we can't be sure last line
                // is not truncated so skip it
                break;
            }
            if (line.size() == 0) continue; // empty lines are not interesting
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

}// ns detail
