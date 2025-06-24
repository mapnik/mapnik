/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/csv/csv_types.hpp>

// std
#include <iosfwd>
#include <string>
#include <vector>

namespace csv_utils {

mapnik::csv_line parse_line(char const* start, char const* end, char separator, char quote, std::size_t num_columns);
mapnik::csv_line parse_line(std::string const& line_str, char separator, char quote);

bool is_likely_number(std::string const& value);

bool ignore_case_equal(std::string const& s0, std::string const& s1);

struct geometry_column_locator
{
    geometry_column_locator()
        : type(UNKNOWN)
        , index(-1)
        , index2(-1)
    {}

    enum { UNKNOWN = 0, WKT, GEOJSON, LON_LAT } type;
    std::size_t index;
    std::size_t index2;
};

mapnik::geometry::geometry<double> extract_geometry(std::vector<std::string> const& row,
                                                    geometry_column_locator const& locator);

template<typename Feature, typename Headers, typename Values, typename Locator, typename Transcoder>
void process_properties(Feature& feature,
                        Headers const& headers,
                        Values const& values,
                        Locator const& locator,
                        Transcoder const& tr)
{
    auto val_beg = values.begin();
    auto val_end = values.end();
    auto num_headers = headers.size();
    for (std::size_t i = 0; i < num_headers; ++i)
    {
        std::string const& fld_name = headers.at(i);
        if (val_beg == val_end)
        {
            feature.put(fld_name, tr.transcode(""));
            continue;
        }
        std::string value = mapnik::util::trim_copy(*val_beg++);
        int value_length = value.length();

        if (locator.index == i &&
            (locator.type == geometry_column_locator::WKT || locator.type == geometry_column_locator::GEOJSON))
            continue;

        bool matched = false;
        bool has_dot = value.find(".") != std::string::npos;
        if (value.empty() || (value_length > 20) || (value_length > 1 && !has_dot && value[0] == '0'))
        {
            matched = true;
            feature.put(fld_name, std::move(tr.transcode(value.c_str())));
        }
        else if (csv_utils::is_likely_number(value))
        {
            bool has_e = value.find("e") != std::string::npos;
            if (has_dot || has_e)
            {
                double float_val = 0.0;
                if (mapnik::util::string2double(value, float_val))
                {
                    matched = true;
                    feature.put(fld_name, float_val);
                }
            }
            else
            {
                mapnik::value_integer int_val = 0;
                if (mapnik::util::string2int(value, int_val))
                {
                    matched = true;
                    feature.put(fld_name, int_val);
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
                feature.put(fld_name, std::move(tr.transcode(value.c_str())));
            }
        }
    }
}

struct csv_file_parser
{
    template<typename T>
    void parse_csv_and_boxes(std::istream& csv_file, T& boxes);

    virtual void add_feature(mapnik::value_integer index, mapnik::csv_line const& values);

    std::vector<std::string> headers_;
    std::string manual_headers_;
    geometry_column_locator locator_;
    mapnik::box2d<double> extent_;
    mapnik::value_integer row_limit_ = 0;
    char separator_ = '\0';
    char quote_ = '\0';
    bool strict_ = false;
    bool extent_initialized_ = false;
    bool has_disk_index_ = false;
};

} // namespace csv_utils

#endif // MAPNIK_CSV_UTILS_DATASOURCE_HPP
