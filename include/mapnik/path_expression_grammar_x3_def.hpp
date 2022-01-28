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

#ifndef MAPNIK_PATH_EXPRESSIONS_GRAMMAR_X3_DEF_HPP
#define MAPNIK_PATH_EXPRESSIONS_GRAMMAR_X3_DEF_HPP
// mapnik
#include <mapnik/path_expression_grammar_x3.hpp>
#include <mapnik/attribute.hpp>

namespace mapnik {
namespace grammar {

namespace x3 = boost::spirit::x3;
using x3::lexeme;
using x3::standard_wide::char_;
auto create_string = [](auto& ctx) {
    _val(ctx).push_back(_attr(ctx));
};
auto create_attribute = [](auto& ctx) {
    _val(ctx).push_back(mapnik::attribute(_attr(ctx)));
};
// rules
x3::rule<class attr_expression, std::string> const attr_expression("attribute");
x3::rule<class str_expression, std::string> const str_expression("string");

auto const attr_expression_def = +(char_ - ']');
auto const str_expression_def = lexeme[+(char_ - '[')];
auto const path_expression_def = *(str_expression[create_string] | ('[' > attr_expression[create_attribute] > ']'));

BOOST_SPIRIT_DEFINE(path_expression, attr_expression, str_expression);

} // namespace grammar
} // namespace mapnik

#endif // MAPNIK_PATH_EXPRESSIONS_GRAMMAR_X3_DEF_HPP
