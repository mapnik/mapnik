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
#include <mapnik/json/positions_grammar.hpp>

// boost
#include <boost/spirit/include/qi_omit.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
// stl
#include <iostream>
#include <string>

namespace mapnik { namespace json {

template <typename Iterator, typename ErrorHandler>
positions_grammar<Iterator, ErrorHandler>::positions_grammar()
    : positions_grammar::base_type(coords,"coordinates")
{
    qi::lit_type lit;
    qi::double_type double_;
    qi::_val_type _val;
    qi::_1_type _1;
    qi::_2_type _2;
    qi::_3_type _3;
    qi::_4_type _4;
    qi::omit_type omit;
    using qi::fail;
    using qi::on_error;

    coords = rings_array[_val = _1] | rings [_val = _1] | ring[_val = _1] |  pos[set_position(_val,_1)]
        ;
    pos = lit('[') > -(double_ > lit(',') > double_) > omit[*(lit(',') > double_)] > lit(']')
        ;
    ring = lit('[') >> pos[push_position(_val,_1)] % lit(',') > lit(']')
        ;
    rings = lit('[') >> ring % lit(',') > lit(']')
        ;
    rings_array = lit('[') >> rings % lit(',') > lit(']')
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
