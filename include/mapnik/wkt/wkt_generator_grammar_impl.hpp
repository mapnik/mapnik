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
#include <mapnik/wkt/wkt_generator_grammar.hpp>
#include <mapnik/geometry_fusion_adapted.hpp>

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
        lit("POINT EMPTY FAIL") // special case for geometry_empty
        ;

    point = lit("POINT(") << point_coord << lit(")")
        ;

    linestring = lit("LINESTRING") << (linestring_coord | lit(" EMPTY"))
        ;

    polygon = lit("POLYGON") << (polygon_coord | lit(" EMPTY"))
        ;

    multi_point = lit("MULTIPOINT") << (multi_point_coord | lit(" EMPTY"))
        ;

    multi_linestring = lit("MULTILINESTRING") << (multi_linestring_coord | lit(" EMPTY"))
        ;

    multi_polygon = lit("MULTIPOLYGON") << (multi_polygon_coord | lit(" EMPTY"))
        ;

    geometry_collection = lit("GEOMETRYCOLLECTION") << (lit("(") << geometries  << lit(")") | lit(" EMPTY"))
        ;

    point_coord = coordinate << lit(' ') << coordinate
        ;

    linestring_coord = lit('(') << point_coord % lit(',') << lit(')')
        ;

    linear_ring_coord = lit('(') << point_coord % lit(',') << lit(')')
        ;

    polygon_coord = lit('(') << linear_ring_coord << *(lit(',') << linear_ring_coord)<< lit(')');
        ;

    multi_point_coord = lit('(') << point_coord % lit(',') << lit(')')
        ;

    multi_linestring_coord = lit("(") << linestring_coord % lit(',') << lit(")")
        ;

    multi_polygon_coord = lit("(") << polygon_coord % lit(',') << lit(")")
        ;

    geometries = geometry % lit(',')
        ;

}

}}
