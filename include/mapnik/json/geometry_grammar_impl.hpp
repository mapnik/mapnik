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
#include <mapnik/config.hpp>
#include <mapnik/json/error_handler.hpp>
#include <mapnik/json/geometry_grammar.hpp>
#include <mapnik/geometry_fusion_adapted.hpp>
// boost
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_function.hpp>

namespace mapnik { namespace json {

template <typename Iterator, typename ErrorHandler >
geometry_grammar<Iterator, ErrorHandler>::geometry_grammar()
    : geometry_grammar::base_type(start,"geometry"),
      coordinates(error_handler)
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
    qi::_r1_type _r1;
    qi::_r2_type _r2;
    qi::_r3_type _r3;
    qi::eps_type eps;
    using qi::fail;
    using qi::on_error;
    using phoenix::push_back;

    start = geometry.alias() | lit("null");

    geometry = lit('{')[_a = 0]
        > (geometry_part(_a, _b, _val) % lit(','))[create_geometry(_val, _a, _b)]
        > lit('}');

    geometry_part = ((lit("\"type\"") > lit(':') > geometry_type_dispatch[_r1 = _1])
                     |
                     (lit("\"coordinates\"") > lit(':') > coordinates[_r2 = _1])
                     |
                     (lit("\"geometries\"") > lit(':') > lit('[') > geometry_collection[_r3 = _1] > lit(']'))
                     |
                     json_.key_value)
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
    auto error_handler_function = boost::phoenix::function<ErrorHandler>(error_handler);
    on_error<fail>(start, error_handler_function(_1, _2, _3, _4));
}

}}
