/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2016 Artem Pavlenko
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
#include <mapnik/feature_factory.hpp>

namespace mapnik { namespace json {

bool from_geojson(std::string const& json, mapnik::geometry::geometry<double> & geom)
{
    try
    {
        mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx, -1)); // temp geometry holder
        const char* start = json.c_str();
        const char* end = start + json.length();
        mapnik::json::parse_geometry(start, end, *feature);
        geom = std::move(feature->get_geometry());
    }
    catch (...)
    {
        return false;
    }
    return true;
}

}}
