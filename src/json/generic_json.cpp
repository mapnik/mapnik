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

#include <mapnik/json/generic_json.hpp>

namespace mapnik { namespace json {

template <typename Iterator>
generic_json<Iterator>::generic_json()
    : generic_json::base_type(value)
{
    qi::lit_type lit;
    qi::_val_type _val;
    qi::_1_type _1;
    using phoenix::construct;
    // generic json types
    value = object | array | string_ | number
        ;

    key_value = string_ > lit(':') > value
        ;

    object = lit('{')
        > -(key_value % lit(','))
        > lit('}')
        ;

    array = lit('[')
        > -(value % lit(','))
        > lit(']')
        ;

    number = strict_double[_val = double_converter(_1)]
        | int__[_val = integer_converter(_1)]
        | lit("true") [_val = true]
        | lit ("false") [_val = false]
        | lit("null")[_val = construct<value_null>()]
        ;
}

}}

using iterator_type = char const*;
template struct mapnik::json::generic_json<iterator_type>;
