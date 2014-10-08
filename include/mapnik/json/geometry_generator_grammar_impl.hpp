/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
  : geometry_generator_grammar::base_type(coordinates)
{
    boost::spirit::karma::uint_type uint_;
    boost::spirit::bool_type bool_;
    boost::spirit::karma::_val_type _val;
    boost::spirit::karma::_1_type _1;
    boost::spirit::karma::lit_type lit;
    boost::spirit::karma::_a_type _a;
    boost::spirit::karma::_r1_type _r1;
    boost::spirit::karma::eps_type eps;
    boost::spirit::karma::string_type kstring;

    coordinates =  point | linestring | polygon
        ;

    point = &uint_(mapnik::geometry_type::types::Point)[_1 = _type(_val)]
        << point_coord [_1 = _first(_val)]
        ;

    linestring = &uint_(mapnik::geometry_type::types::LineString)[_1 = _type(_val)]
        << lit('[')
        << coords
        << lit(']')
        ;

    polygon = &uint_(mapnik::geometry_type::types::Polygon)[_1 = _type(_val)]
        << lit('[')
        << coords2
        << lit("]]")
        ;

    point_coord = &uint_
        << lit('[')
        << coordinate << lit(',') << coordinate
        << lit(']')
        ;

    polygon_coord %= ( &uint_(mapnik::SEG_MOVETO) << eps[_r1 += 1]
                       << kstring[ if_ (_r1 > 1u) [_1 = "],["]
                                         .else_[_1 = '[' ]]
                       |
                       &uint_(mapnik::SEG_LINETO)
                       << lit(',')) << lit('[') << coordinate << lit(',') << coordinate << lit(']')
        ;

    coords2 %= *polygon_coord(_a)
        ;

    coords = point_coord % lit(',')
        ;
}

template <typename OutputIterator, typename GeometryContainer>
multi_geometry_generator_grammar<OutputIterator, GeometryContainer>::multi_geometry_generator_grammar()
  : multi_geometry_generator_grammar::base_type(start)
{
    boost::spirit::karma::uint_type uint_;
    boost::spirit::bool_type bool_;
    boost::spirit::karma::_val_type _val;
    boost::spirit::karma::_1_type _1;
    boost::spirit::karma::lit_type lit;
    boost::spirit::karma::_a_type _a;
    boost::spirit::karma::eps_type eps;
    boost::spirit::karma::string_type kstring;

    geometry_types.add
        (mapnik::geometry_type::types::Point,"\"Point\"")
        (mapnik::geometry_type::types::LineString,"\"LineString\"")
        (mapnik::geometry_type::types::Polygon,"\"Polygon\"")
        (mapnik::geometry_type::types::Point + 3,"\"MultiPoint\"")
        (mapnik::geometry_type::types::LineString + 3,"\"MultiLineString\"")
        (mapnik::geometry_type::types::Polygon + 3,"\"MultiPolygon\"")
        ;

    start %= ( eps(boost::phoenix::at_c<1>(_a))[_a = multi_type_(_val)]
               << lit("{\"type\":\"GeometryCollection\",\"geometries\":[")
               << geometry_collection << lit("]}")
               |
               geometry)
        ;

    geometry_collection = -(geometry2 % lit(','))
        ;

    geometry = ( &bool_(true)[_1 = not_empty_(_val)] << lit("{\"type\":")
                 << geometry_types[_1 = boost::phoenix::at_c<0>(_a)][_a = multi_type_(_val)]
                 << lit(",\"coordinates\":")
                 << kstring[ boost::phoenix::if_ (boost::phoenix::at_c<0>(_a) > 3u) [_1 = '['].else_[_1 = ""]]
                 << coordinates
                 << kstring[ boost::phoenix::if_ (boost::phoenix::at_c<0>(_a) > 3u) [_1 = ']'].else_[_1 = ""]]
                 << lit('}')) | lit("null")
        ;

    geometry2 = lit("{\"type\":")
        << geometry_types[_1 = _a][_a = type_(_val)]
        << lit(",\"coordinates\":")
        << path
        << lit('}')
        ;

    coordinates %= path % lit(',')
        ;
}

}}
