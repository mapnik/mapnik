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

#ifndef MAPNIK_JSON_GENERIC_JSON_GRAMMAR_X3_DEF_HPP
#define MAPNIK_JSON_GENERIC_JSON_GRAMMAR_X3_DEF_HPP

#include <boost/fusion/include/std_pair.hpp>
#include <mapnik/json/generic_json_grammar_x3.hpp>
#include <mapnik/json/unicode_string_grammar_x3.hpp>

namespace mapnik {
namespace json {
namespace grammar {

namespace x3 = boost::spirit::x3;

namespace {

const auto make_null = [](auto const& ctx) {
    _val(ctx) = mapnik::value_null{};
};

const auto make_true = [](auto const& ctx) {
    _val(ctx) = true;
};

const auto make_false = [](auto const& ctx) {
    _val(ctx) = false;
};

const auto assign = [](auto const& ctx) {
    _val(ctx) = std::move(_attr(ctx));
};

const auto assign_key = [](auto const& ctx) {
    std::get<0>(_val(ctx)) = std::move(_attr(ctx));
};

const auto assign_value = [](auto const& ctx) {
    std::get<1>(_val(ctx)) = std::move(_attr(ctx));
};

} // namespace

using x3::lit;

// import unicode string rule
const auto json_string = mapnik::json::grammar::unicode_string;

// rules
x3::rule<class json_object_tag, json_object> const object("JSON Object");
x3::rule<class json_array_tag, json_array> const array("JSON Array");
x3::rule<class json_number_tag, json_value> const number("JSON Number");

auto const json_double = x3::real_parser<value_double, x3::strict_real_policies<value_double>>();
auto const json_integer = x3::int_parser<value_integer, 10, 1, -1>();

// generic json types
auto const value_def = object | array | json_string | number;

auto const key_value_def = json_string[assign_key] > lit(':') > value[assign_value];

auto const object_def = lit('{')                  //
                        > -(key_value % lit(',')) //
                        > lit('}')                //
  ;

auto const array_def = lit('[')              //
                       > -(value % lit(',')) //
                       > lit(']')            //
  ;

auto const number_def = json_double[assign]        //
                        | json_integer[assign]     //
                        | lit("true")[make_true]   //
                        | lit("false")[make_false] //
                        | lit("null")[make_null]   //
  ;

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>

BOOST_SPIRIT_DEFINE(value, object, key_value, array, number);

MAPNIK_DISABLE_WARNING_POP

} // namespace grammar
} // namespace json
} // namespace mapnik

#endif // MAPNIK_JSON_GENERIC_JSON_GRAMMAR_X3_DEF_HPP
