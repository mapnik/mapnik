/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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
#include <mapnik/json/geometry_grammar.hpp>

// boost
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <iostream>                     // for clog, endl, etc
#include <string>                       // for string

namespace mapnik { namespace json {

template <typename Iterator>
geometry_grammar<Iterator>::geometry_grammar()
    : geometry_grammar::base_type(geometry,"geometry")
{

    qi::lit_type lit;
    qi::int_type int_;
    qi::double_type double_;
    qi::_1_type _1;
    qi::_2_type _2;
    qi::_3_type _3;
    qi::_4_type _4;
    qi::_a_type _a;
    qi::_r1_type _r1;
    qi::_r2_type _r2;
    qi::eps_type eps;
    qi::_pass_type _pass;
    using qi::fail;
    using qi::on_error;
    using boost::phoenix::new_;
    using boost::phoenix::push_back;
    using boost::phoenix::construct;
   // Nabialek trick - FIXME: how to bind argument to dispatch rule?
    // geometry = lit("\"geometry\"")
    //    >> lit(':') >> lit('{')
    //    >> lit("\"type\"") >> lit(':') >> geometry_dispatch[_a = _1]
    //    >> lit(',') >> lit("\"coordinates\"") >> lit(':')
    //    >> qi::lazy(*_a)
    //    >> lit('}')
    //    ;
    // geometry_dispatch.add
    //    ("\"Point\"",&point_coordinates)
    //    ("\"LineString\"",&linestring_coordinates)
    //    ("\"Polygon\"",&polygon_coordinates)
    //    ;
    //////////////////////////////////////////////////////////////////

    geometry = (lit('{')[_a = 0 ]
                >> lit("\"type\"") >> lit(':') >> geometry_dispatch[_a = _1] // <---- should be Nabialek trick!
                >> lit(',')
                >> (lit("\"coordinates\"") > lit(':') > coordinates(_r1,_a)
                    |
                    lit("\"geometries\"") > lit(':')
                    >> lit('[') >> geometry_collection(_r1) >> lit(']'))
                >> lit('}'))
        | lit("null")
        ;

    geometry_dispatch.add
        ("\"Point\"",1)
        ("\"LineString\"",2)
        ("\"Polygon\"",3)
        ("\"MultiPoint\"",4)
        ("\"MultiLineString\"",5)
        ("\"MultiPolygon\"",6)
        ("\"GeometryCollection\"",7)
        //
        ;

    coordinates = (eps(_r2 == 1) > point_coordinates(_r1))
        | (eps(_r2 == 2) > linestring_coordinates(_r1))
        | (eps(_r2 == 3) > polygon_coordinates(_r1))
        | (eps(_r2 == 4) > multipoint_coordinates(_r1))
        | (eps(_r2 == 5) > multilinestring_coordinates(_r1))
        | (eps(_r2 == 6) > multipolygon_coordinates(_r1))
        ;

    point_coordinates =  eps[ _a = new_<geometry_type>(geometry_type::types::Point) ]
        > ( point(SEG_MOVETO,_a) [push_back(_r1,_a)] | eps[cleanup_(_a)][_pass = false] )
        ;

    linestring_coordinates = eps[ _a = new_<geometry_type>(geometry_type::types::LineString)]
        > -(points(_a) [push_back(_r1,_a)]
           | eps[cleanup_(_a)][_pass = false])
        ;

    polygon_coordinates = eps[ _a = new_<geometry_type>(geometry_type::types::Polygon) ]
        > ((lit('[')
            > -(points(_a)[close_path_(_a)] % lit(','))
            > lit(']')) [push_back(_r1,_a)]
           | eps[cleanup_(_a)][_pass = false])
        ;

    multipoint_coordinates =  lit('[')
        > -(point_coordinates(_r1) % lit(','))
        > lit(']')
        ;

    multilinestring_coordinates =  lit('[')
        > -(linestring_coordinates(_r1) % lit(','))
        > lit(']')
        ;

    multipolygon_coordinates =  lit('[')
        > -(polygon_coordinates(_r1) % lit(','))
        > lit(']')
        ;

    geometry_collection = *geometry(_r1) >> *(lit(',') >> geometry(_r1))
        ;

    // point
    point = lit('[') > -((double_ > lit(',') > double_)[push_vertex_(_r1,_r2,_1,_2)]) > lit(']');
    // points
    points = lit('[')[_a = SEG_MOVETO] > -(point (_a,_r1) % lit(',')[_a = SEG_LINETO]) > lit(']');

    // give some rules names
    geometry.name("Geometry");
    geometry_collection.name("GeometryCollection");
    geometry_dispatch.name("Geometry dispatch");
    coordinates.name("Coordinates");
    // error handler
    on_error<fail>
        (
            geometry
            , std::clog
            << boost::phoenix::val("Error! Expecting ")
            << _4  // what failed?
            << boost::phoenix::val(" here: \"")
            << where_message_(_3, _2, 16) // max 16 chars
            << boost::phoenix::val("\"")
            << std::endl
            );
}

template struct mapnik::json::geometry_grammar<std::string::const_iterator>;
template struct mapnik::json::geometry_grammar<boost::spirit::multi_pass<std::istreambuf_iterator<char> > >;

}}
