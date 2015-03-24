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
#include <mapnik/json/geometry_generator_grammar.hpp>
#include <mapnik/util/spirit_transform_attribute.hpp>

// boost
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/fusion/include/at.hpp>

namespace mapnik { namespace json {

namespace karma = boost::spirit::karma;
namespace phoenix = boost::phoenix;

template <typename OutputIterator, typename Geometry>
geometry_generator_grammar<OutputIterator, Geometry>::geometry_generator_grammar()
    : geometry_generator_grammar::base_type(geometry)
{
    boost::spirit::karma::_val_type _val;
    boost::spirit::karma::_1_type _1;
    boost::spirit::karma::_a_type _a;
    boost::spirit::karma::lit_type lit;
    boost::spirit::karma::uint_type uint_;
    boost::spirit::karma::eps_type eps;

    geometry = geometry_dispatch.alias()
        ;

    geometry_dispatch = eps[_a = geometry_type(_val)] <<
        (&uint_(geometry::geometry_types::Point)[_1 = _a]
         << (point | lit("null")))
        |
        (&uint_(geometry::geometry_types::LineString)[_1 = _a]
         << (linestring | lit("null")))
        |
        (&uint_(geometry::geometry_types::Polygon)[_1 = _a]
         << (polygon | lit("null")))
        |
        (&uint_(geometry::geometry_types::MultiPoint)[_1 = _a]
         << (multi_point | lit("null")))
        |
        (&uint_(geometry::geometry_types::MultiLineString)[_1 = _a]
         << (multi_linestring | lit("null")))
        |
        (&uint_(geometry::geometry_types::MultiPolygon)[_1 = _a]
         << (multi_polygon | lit("null")))
        |
        (&uint_(geometry::geometry_types::GeometryCollection)[_1 = _a]
         << (geometry_collection | lit("null")))
        |
        lit("null")
        ;

    point = lit("{\"type\":\"Point\",\"coordinates\":") << point_coord << lit("}")
        ;
    linestring = lit("{\"type\":\"LineString\",\"coordinates\":[") << linestring_coord << lit("]}")
        ;
    polygon = lit("{\"type\":\"Polygon\",\"coordinates\":[") << polygon_coord << lit("]}")
        ;
    multi_point = lit("{\"type\":\"MultiPoint\",\"coordinates\":[") << multi_point_coord << lit("]}")
        ;
    multi_linestring = lit("{\"type\":\"MultiLineString\",\"coordinates\":[") << multi_linestring_coord << lit("]}")
        ;
    multi_polygon = lit("{\"type\":\"MultiPolygon\",\"coordinates\":[") << multi_polygon_coord << lit("]}")
        ;
    geometry_collection = lit("{\"type\":\"GeometryCollection\",\"geometries\":[") << geometries  << lit("]}")
        ;
    point_coord = lit('[') << coordinate << lit(',') << coordinate  << lit(']')
        ;
    linestring_coord = point_coord % lit(',')
        ;
    polygon_coord = lit('[') << exterior_ring_coord << lit(']') << interior_ring_coord
        ;
    exterior_ring_coord = linestring_coord.alias()
        ;
    interior_ring_coord =  *(lit(",[") << exterior_ring_coord << lit(']'))
        ;
    multi_point_coord = linestring_coord.alias()
        ;
    multi_linestring_coord = (lit('[') << linestring_coord << lit(']')) % lit(',')
        ;
    multi_polygon_coord = (lit('[') << polygon_coord << lit(']')) % lit(',')
        ;
    geometries =  geometry % lit(',')
        ;
}

}}
