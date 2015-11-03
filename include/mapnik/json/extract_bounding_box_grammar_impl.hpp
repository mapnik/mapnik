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
#include <mapnik/json/extract_bounding_box_grammar.hpp>

// boost
#include <boost/spirit/include/qi_omit.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/repository/include/qi_iter_pos.hpp>
// stl
#include <iostream>
#include <string>

namespace mapnik { namespace json {

namespace repo = boost::spirit::repository;

template <typename Iterator, typename ErrorHandler>
extract_bounding_box_grammar<Iterator, ErrorHandler>::extract_bounding_box_grammar()
    : extract_bounding_box_grammar::base_type(start,"bounding boxes")
{
    qi::lit_type lit;
    qi::double_type double_;
    qi::_val_type _val;
    qi::_1_type _1;
    qi::_2_type _2;
    qi::_3_type _3;
    qi::_4_type _4;
    qi::omit_type omit;
    qi::_r1_type _r1;
    qi::_r2_type _r2;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::eps_type eps;
    qi::raw_type raw;
    qi::char_type char_;
    qi::no_skip_type no_skip;
    boost::spirit::repository::qi::iter_pos_type iter_pos;
    using qi::fail;
    using qi::on_error;

    start = features(_r1)
        ;

    features = no_skip[iter_pos[_a = _1]] >> -(lit('{')
                                               >> *((json.key_value - lit("\"features\"")) >> lit(','))
                                               >> lit("\"features\"")
                                               >> lit(':'))
                                          >> lit('[') >> (feature(_r1,_a) % lit(',')) >> lit(']')
        ;

    feature = raw[lit('{')[_a = 1]
                  >> *(eps(_a > 0) >> (
                           lit("\"FeatureCollection\"") > eps(false) // fail if nested FeatureCollection
                           |
                           lit('{')[_a += 1]
                           |
                           lit('}')[_a -= 1]
                           |
                           coords[_b = _1]
                           |
                           json.string_
                           |
                           char_))][push_box(_r1, _r2, _b, _1)]
        ;

    coords = lit("\"coordinates\"")
        >> lit(':') >> (rings_array(_a) | rings (_a) | ring(_a) | pos[calculate_bounding_box(_a,_1)])[_val = _a]
        ;

    pos = lit('[') > -(double_ > lit(',') > double_) > omit[*(lit(',') > double_)] > lit(']')
        ;

    ring = lit('[') >> pos[calculate_bounding_box(_r1,_1)] % lit(',') > lit(']')
        ;

    rings = lit('[') >> ring(_r1)  % lit(',') > lit(']')
        ;

    rings_array = lit('[') >> rings(_r1) % lit(',') > lit(']')
        ;

    // generic json types
    json.value = json.object | json.array | json.string_ | json.number
        ;

    json.pairs = json.key_value % lit(',')
        ;

    json.key_value = (json.string_ >> lit(':') >> json.value)
        ;

    json.object = lit('{') >> *json.pairs >> lit('}')
        ;

    json.array = lit('[')
        >> json.value >> *(lit(',') >> json.value)
        >> lit(']')
        ;

    json.number = json.strict_double
        | json.int__
        | lit("true")
        | lit("false")
        | lit("null")
        ;

    coords.name("Coordinates");
    pos.name("Position");
    ring.name("Ring");
    rings.name("Rings");
    rings_array.name("Rings array");

    // error handler
    on_error<fail>(coords, error_handler(_1, _2, _3, _4));
}

}}
