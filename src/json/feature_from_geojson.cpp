/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include <mapnik/json/geometry_parser.hpp>
#include <mapnik/json/parse_feature.hpp>

namespace mapnik {
namespace json {

bool from_geojson(std::string const& json, feature_impl& feature)
{
    try
    {
        const char* start = json.c_str();
        const char* end = start + json.length();
        mapnik::json::parse_feature(start, end, feature);
    } catch (...)
    {
        return false;
    }
    return true;
}

} // namespace json
} // namespace mapnik
