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

// boost
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/fusion/include/at.hpp>

namespace mapnik { namespace json {

namespace karma = boost::spirit::karma;

template <typename OutputIterator, typename Geometry>
geometry_generator_grammar<OutputIterator, Geometry>::geometry_generator_grammar()
    : geometry_generator_grammar::base_type(start)
{
    boost::spirit::karma::_val_type _val;
    boost::spirit::karma::_1_type _1;
    boost::spirit::karma::_a_type _a;
    boost::spirit::karma::_r1_type _r1;
    boost::spirit::karma::_r2_type _r2;
    boost::spirit::karma::lit_type lit;
    boost::spirit::karma::uint_type uint_;

    geometry_types.add
        (mapnik::new_geometry::geometry_types::Point,"\"Point\"")
        (mapnik::new_geometry::geometry_types::LineString,"\"LineString\"")
        (mapnik::new_geometry::geometry_types::Polygon,"\"Polygon\"")
        (mapnik::new_geometry::geometry_types::MultiPoint,"\"MultiPoint\"")
        (mapnik::new_geometry::geometry_types::MultiLineString,"\"MultiLineString\"")
        (mapnik::new_geometry::geometry_types::MultiPolygon,"\"MultiPolygon\"")
        (mapnik::new_geometry::geometry_types::GeometryCollection,"\"GeometryCollection\"")
        ;

    start = geometry.alias()
        ;

    geometry = lit("{\"type\":")
        << geometry_types[_1 = _a][_a = geometry_type(_val)]
        << lit(",\"coordinates\":")
        << coordinates(_a,_val)
        << lit('}')
        ;

    coordinates = (&uint_(new_geometry::geometry_types::Point)[_1 = _r1] << point[_1 = extract_point(_r2)])
        |
        (&uint_(new_geometry::geometry_types::LineString)[_1 = _r1] << linestring[_1 = extract_linestring(_r2)])
        |
        (&uint_(new_geometry::geometry_types::Polygon)[_1 = _r1] << polygon[_1 = extract_polygon(_r2)])
        |
        (&uint_(new_geometry::geometry_types::MultiPoint)[_1 = _r1] << multi_point[_1 = extract_multipoint(_r2)])
        |
        (&uint_(new_geometry::geometry_types::MultiLineString)[_1 = _r1] << multi_linestring[_1 = extract_multilinestring(_r2)])
        |
        (&uint_(new_geometry::geometry_types::MultiPolygon)[_1 = _r1] << multi_polygon[_1 = extract_multipolygon(_r2)])
        //|
        //(&uint_(new_geometry::geometry_types::GeometryCollection)[_1 = _r1] << geometry_collection)[_1 = extract_collection(_r2)])
        ;

    point = lit('[') << coordinate[_1 = get_x(_val)] << lit(',') << coordinate[_1 = get_y(_val)] << lit(']')
        ;

    linestring = lit('[') << point % lit(',') << lit(']')
        ;

    polygon = lit('[') << linestring << *(lit(',') << linestring) << lit(']')
        ;

    multi_point = lit('[') << point % lit(',') << lit(']')
        ;

    multi_linestring = lit('[') << linestring % lit(',') << lit(']')
        ;

    multi_polygon = lit('[') << polygon % lit(',') << lit(']')
        ;

    geometry_collection = lit("{\"type\":\"GeometryCollection\",\"geometries\":[") /*<< geometry % lit(',')*/ << lit("]}")
        ;
}

}}
