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
#include <mapnik/wkt/wkt_generator_grammar.hpp>
#include <mapnik/geometry/fusion_adapted.hpp>
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/phoenix.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace wkt {

template <typename OutputIterator, typename Geometry>
wkt_generator_grammar<OutputIterator, Geometry>::wkt_generator_grammar()
    : wkt_generator_grammar::base_type(geometry)
{
    boost::spirit::karma::lit_type lit;
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
        lit("POINT EMPTY") // special case for geometry_empty
        ;

    point = lit("POINT(") << coordinate << lit(' ') << coordinate << lit(")")
        ;
    linestring = lit("LINESTRING(") << linestring_coord << lit(")")
        ;
    polygon = lit("POLYGON(") << polygon_coord << lit(")")
        ;
    multi_point = lit("MULTIPOINT(") << multi_point_coord << lit(")")
        ;
    multi_linestring = lit("MULTILINESTRING(") << multi_linestring_coord << lit(")")
        ;
    multi_polygon = lit("MULTIPOLYGON(") << multi_polygon_coord << lit(")")
        ;
    geometry_collection = lit("GEOMETRYCOLLECTION(") << geometries  << lit(")")
        ;
    point_coord = coordinate << lit(' ') << coordinate
        ;
    linestring_coord = point_coord % lit(',')
        ;
    polygon_coord = lit('(') << exterior_ring_coord << lit(')') << interior_ring_coord
        ;
    exterior_ring_coord = linestring_coord.alias()
        ;
    interior_ring_coord =  *(lit(",(") << exterior_ring_coord << lit(')'))
        ;
    multi_point_coord = linestring_coord.alias()
        ;
    multi_linestring_coord = (lit('(') << linestring_coord  << lit(')')) % lit(',')
        ;
    multi_polygon_coord = (lit('(') << polygon_coord << lit(')')) % lit(',')
        ;
    geometries = geometry % lit(',')
        ;

}

}}
