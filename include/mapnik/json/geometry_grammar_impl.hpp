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
#include <mapnik/config.hpp>
#include <mapnik/json/error_handler.hpp>
#include <mapnik/json/geometry_grammar.hpp>
#include <mapnik/json/positions_grammar_impl.hpp>

// boost
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <iostream>                     // for clog, endl, etc
#include <string>                       // for string

namespace mapnik { namespace json {

template <typename Iterator, typename ErrorHandler >
geometry_grammar<Iterator, ErrorHandler>::geometry_grammar()
    : geometry_grammar::base_type(start,"geometry")
{
    qi::lit_type lit;
    qi::int_type int_;
    qi::double_type double_;
    qi::_val_type _val;
    qi::_1_type _1;
    qi::_2_type _2;
    qi::_3_type _3;
    qi::_4_type _4;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::eps_type eps;
    using qi::fail;
    using qi::on_error;
    using phoenix::push_back;

    start = geometry.alias(); // | geometry_collection;

    geometry = (lit('{')[_a = 0 ]
                >> (-lit(',') >> (lit("\"type\"") >> lit(':') >> geometry_type_dispatch[_a = _1])
                    ^
                    (-lit(',') >> (lit("\"coordinates\"") > lit(':') > coordinates[_b = _1]))
                    ^
                    (-lit(',') >> lit("\"geometries\"") >> lit(':') >> lit('[') >> geometry_collection[_val = _1] >> lit(']')))[create_geometry(_val,_a,_b)]
                >> lit('}'))
        | lit("null")
        ;

    geometry_collection = geometry[push_back(_val, _1)] % lit(',')
        ;
    geometry_type_dispatch.add
        ("\"Point\"",1)
        ("\"LineString\"",2)
        ("\"Polygon\"",3)
        ("\"MultiPoint\"",4)
        ("\"MultiLineString\"",5)
        ("\"MultiPolygon\"",6)
        ("\"GeometryCollection\"",7)
        ;

    // give some rules names
    geometry.name("Geometry");
    geometry_collection.name("GeometryCollection");
    geometry_type_dispatch.name("type: (Point|LineString|Polygon|MultiPoint|MultiLineString|MultiPolygon|GeometryCollection)");
    coordinates.name("coordinates");
    // error handler
    on_error<fail>(start, error_handler(_1, _2, _3, _4));
}

}}
