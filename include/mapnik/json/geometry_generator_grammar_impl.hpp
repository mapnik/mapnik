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
#include <mapnik/json/geometry_generator_grammar.hpp>
#include <mapnik/geometry/fusion_adapted.hpp>

namespace mapnik {
namespace json {

namespace karma = boost::spirit::karma;

template<typename OutputIterator, typename Geometry>
geometry_generator_grammar<OutputIterator, Geometry>::geometry_generator_grammar()
    : geometry_generator_grammar::base_type(geometry)
{
    boost::spirit::karma::lit_type lit;
    // clang-format off
    geometry =
        point
        |
        linestring
        |
        polygon
        |
        multi_point
        |
        multi_linestring
        |
        multi_polygon
        |
        geometry_collection
        |
        lit("null") // geometry_empty
        ;

    point = lit("{\"type\":\"Point\",\"coordinates\":") << point_coord << lit("}")
        ;

    linestring = lit("{\"type\":\"LineString\",\"coordinates\":") << linestring_coord << lit("}")
        ;

    polygon = lit("{\"type\":\"Polygon\",\"coordinates\":") << polygon_coord << lit("}")
        ;

    multi_point = lit("{\"type\":\"MultiPoint\",\"coordinates\":") << multi_point_coord << lit("}")
        ;

    multi_linestring = lit("{\"type\":\"MultiLineString\",\"coordinates\":") << multi_linestring_coord << lit("}")
        ;

    multi_polygon = lit("{\"type\":\"MultiPolygon\",\"coordinates\":") << multi_polygon_coord << lit("}")
        ;

    geometry_collection = lit("{\"type\":\"GeometryCollection\",\"geometries\":[") << geometries  << lit("]}")
        ;

    point_coord = lit('[') << coordinate << lit(',') << coordinate  << lit(']')
        ;

    linestring_coord = lit('[') << -(point_coord % lit(',')) << lit(']')
        ;

    linear_ring_coord = lit('[') << -(point_coord % lit(',')) << lit(']')//linestring_coord.alias()
        ;

    polygon_coord = lit('[') << -(linear_ring_coord % lit(',')) << lit(']')
        ;

    multi_point_coord = lit('[') << -(point_coord % lit(',')) << lit(']');//linestring_coord.alias()
        ;

    multi_linestring_coord = lit('[') << -(linestring_coord  % lit(',')) << lit(']')
        ;

    multi_polygon_coord = lit('[') << -(polygon_coord  % lit(',')) << lit("]")
        ;

    geometries = geometry % lit(',')
        ;
    // clang-format on
}

} // namespace json
} // namespace mapnik
