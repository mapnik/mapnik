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

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_correct.hpp>
#include <mapnik/wkt/wkt_factory.hpp>
#include <mapnik/json/geometry_parser.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/csv/csv_grammar.hpp>
#include <mapnik/util/trim.hpp>
// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <boost/algorithm/string.hpp>
#pragma GCC diagnostic pop

#include <string>
#include <cstdio>
#include <algorithm>

namespace csv_utils
{

static const mapnik::csv_line_grammar<char const*> line_g;

template <typename Iterator>
static mapnik::csv_line parse_line(Iterator start, Iterator end, std::string const& separator, std::size_t num_columns)
{
    mapnik::csv_line values;
    if (num_columns > 0) values.reserve(num_columns);
    boost::spirit::standard::blank_type blank;
    if (!boost::spirit::qi::phrase_parse(start, end, (line_g)(boost::phoenix::cref(separator)), blank, values))
    {
        throw std::runtime_error("Failed to parse CSV line:\n" + std::string(start, end));
    }
    return values;
}

static inline mapnik::csv_line parse_line(std::string const& line_str, std::string const& separator)
{
    auto start = line_str.c_str();
    auto end   = start + line_str.length();
    return parse_line(start, end, separator, 0);
}

static inline bool is_likely_number(std::string const& value)
{
    return( strspn( value.c_str(), "e-.+0123456789" ) == value.size() );
}

struct ignore_case_equal_pred
{
    bool operator () (unsigned char a, unsigned char b) const
    {
        return std::tolower(a) == std::tolower(b);
    }
};

inline bool ignore_case_equal(std::string const& s0, std::string const& s1)
{
    return std::equal(s0.begin(), s0.end(),
                      s1.begin(), ignore_case_equal_pred());
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
            throw std::runtime_error("Failed to parse WKT:" + row[locator.index]);
        }
    }
    else if (locator.type == geometry_column_locator::GEOJSON)
    {

        if (!mapnik::json::from_geojson(row[locator.index], geom))
        {
            throw std::runtime_error("Failed to parse GeoJSON:" + row[locator.index]);
        }
    }
    else if (locator.type == geometry_column_locator::LON_LAT)
    {
        double x, y;
        if (!mapnik::util::string2double(row[locator.index],x))
        {
            throw std::runtime_error("Failed to parse Longitude(Easting):" + row[locator.index]);
        }
        if (!mapnik::util::string2double(row[locator.index2],y))
        {
            throw std::runtime_error("Failed to parse Latitude(Northing):" + row[locator.index2]);
        }
        geom = mapnik::geometry::point<double>(x,y);
    }
    return geom;
}

template <typename Feature, typename Headers, typename Values, typename Locator, typename Transcoder>
void process_properties(Feature & feature, Headers const& headers, Values const& values, Locator const& locator, Transcoder const& tr)
{
    auto val_beg = values.begin();
    auto val_end = values.end();
    auto num_headers = headers.size();
    for (std::size_t i = 0; i < num_headers; ++i)
    {
        std::string const& fld_name = headers.at(i);
        if (val_beg == val_end)
        {
            feature.put(fld_name,tr.transcode(""));
            continue;
        }
        std::string value = mapnik::util::trim_copy(*val_beg++);
        int value_length = value.length();

        if (locator.index == i && (locator.type == detail::geometry_column_locator::WKT
                                   || locator.type == detail::geometry_column_locator::GEOJSON)  ) continue;


        bool matched = false;
        bool has_dot = value.find(".") != std::string::npos;
        if (value.empty() ||
            (value_length > 20) ||
            (value_length > 1 && !has_dot && value[0] == '0'))
        {
            matched = true;
            feature.put(fld_name,std::move(tr.transcode(value.c_str())));
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
                    feature.put(fld_name,float_val);
                }
            }
            else
            {
                mapnik::value_integer int_val = 0;
                if (mapnik::util::string2int(value,int_val))
                {
                    matched = true;
                    feature.put(fld_name,int_val);
                }
            }
        }
        if (!matched)
        {
            if (csv_utils::ignore_case_equal(value, "true"))
            {
                feature.put(fld_name, true);
            }
            else if (csv_utils::ignore_case_equal(value, "false"))
            {
                feature.put(fld_name, false);
            }
            else // fallback to string
            {
                feature.put(fld_name,std::move(tr.transcode(value.c_str())));
            }
        }
    }
}


}// ns detail

#endif // MAPNIK_CSV_UTILS_DATASOURCE_HPP
