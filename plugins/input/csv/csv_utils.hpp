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

#ifndef MAPNIK_CSV_UTILS_DATASOURCE_HPP
#define MAPNIK_CSV_UTILS_DATASOURCE_HPP

#include <mapnik/debug.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_correct.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/json/geometry_parser.hpp>
#include <mapnik/util/conversions.hpp>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <boost/algorithm/string.hpp>
#pragma GCC diagnostic pop

#include <string>
#include <cstdio>

namespace csv_utils
{
    static inline bool is_likely_number(std::string const& value)
    {
        return( strspn( value.c_str(), "e-.+0123456789" ) == value.size() );
    }

    static inline void fix_json_quoting(std::string & csv_line)
    {
        std::string wrapping_char;
        std::string::size_type j_idx = std::string::npos;
        std::string::size_type post_idx = std::string::npos;
        std::string::size_type j_idx_double = csv_line.find("\"{");
        std::string::size_type j_idx_single = csv_line.find("'{");
        if (j_idx_double != std::string::npos)
        {
            wrapping_char = "\"";
            j_idx = j_idx_double;
            post_idx = csv_line.find("}\"");

        }
        else if (j_idx_single != std::string::npos)
        {
            wrapping_char = "'";
            j_idx = j_idx_single;
            post_idx = csv_line.find("}'");
        }
        // we are positive it is valid json
        if (!wrapping_char.empty())
        {
            // grab the json chunk
            std::string json_chunk = csv_line.substr(j_idx,post_idx+wrapping_char.size());
            bool does_not_have_escaped_double_quotes = (json_chunk.find("\\\"") == std::string::npos);
            // ignore properly escaped quotes like \" which need no special handling
            if (does_not_have_escaped_double_quotes)
            {
                std::string pre_json = csv_line.substr(0,j_idx);
                std::string post_json = csv_line.substr(post_idx+wrapping_char.size());
                // handle "" in a string wrapped in "
                // http://tools.ietf.org/html/rfc4180#section-2 item 7.
                // e.g. "{""type"":""Point"",""coordinates"":[30.0,10.0]}"
                if (json_chunk.find("\"\"") != std::string::npos)
                {
                    boost::algorithm::replace_all(json_chunk,"\"\"","\\\"");
                    csv_line = pre_json + json_chunk + post_json;
                }
                // handle " in a string wrapped in '
                // e.g. '{"type":"Point","coordinates":[30.0,10.0]}'
                else
                {
                    // escape " because we cannot exchange for single quotes
                    // https://github.com/mapnik/mapnik/issues/1408
                    boost::algorithm::replace_all(json_chunk,"\"","\\\"");
                    boost::algorithm::replace_all(json_chunk,"'","\"");
                    csv_line = pre_json + json_chunk + post_json;
                }
            }
        }
    }
}


namespace detail {

template <typename T>
std::size_t file_length(T & stream)
{
    stream.seekg(0, std::ios::end);
    return stream.tellg();
}

static inline std::string detect_separator(std::string const& str)
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

static inline void locate_geometry_column(std::string const& header, std::size_t index, geometry_column_locator & locator)
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

static mapnik::geometry::geometry<double> extract_geometry(std::vector<std::string> const& row, geometry_column_locator const& locator)
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

}// ns detail

#endif // MAPNIK_CSV_UTILS_DATASOURCE_HPP
