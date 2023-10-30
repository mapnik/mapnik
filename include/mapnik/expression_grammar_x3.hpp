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

#ifndef MAPNIK_EXPRESSIONS_GRAMMAR_X3_HPP
#define MAPNIK_EXPRESSIONS_GRAMMAR_X3_HPP

#include <mapnik/expression_node.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {
namespace grammar {

#if BOOST_VERSION >= 106700
using transcoder_type = mapnik::transcoder;
#else
using transcoder_type = std::reference_wrapper<mapnik::transcoder const>;
#endif

namespace x3 = boost::spirit::x3;
struct transcoder_tag;
struct expression_class; // top-most ID
using expression_grammar_type = x3::rule<expression_class, expr_node>;

expression_grammar_type const expression("expression");

BOOST_SPIRIT_DECLARE(expression_grammar_type);

} // namespace grammar
} // namespace mapnik

#endif // MAPNIK_EXPRESSIONS_GRAMMAR_X3_HPP
