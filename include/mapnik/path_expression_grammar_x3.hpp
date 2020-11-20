/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_PATH_EXPRESSIONS_GRAMMAR_X3_HPP
#define MAPNIK_PATH_EXPRESSIONS_GRAMMAR_X3_HPP

// mapnik
#include <mapnik/path_expression.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik { namespace grammar {

namespace x3 = boost::spirit::x3;
struct path_expression_class; // top-most ID
using path_expression_grammar_type = x3::rule<path_expression_class, path_expression>;

path_expression_grammar_type const path_expression = "path_expression";

BOOST_SPIRIT_DECLARE(path_expression_grammar_type);

}}

#endif  // MAPNIK_PATH_EXPRESSIONS_GRAMMAR_X3_HPP
