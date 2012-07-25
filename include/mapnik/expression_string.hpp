/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_EXPRESSION_STRING_HPP
#define MAPNIK_EXPRESSION_STRING_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/expression_node.hpp>

// stl
#include <string>

namespace mapnik
{
MAPNIK_DECL std::string to_expression_string(expr_node const& node);

/*
The following two templates are intentionally invalid and will prompt
a compile error if ever instantiated. This should prevent accidentally
passing a pointer (either raw or shared) as the argument.  Without them,
the compiler could construct a temporary expr_node(bool) using
implicit pointer-to-bool conversion, thus any non-null pointer
would yield "true".
*/

template <typename T>
std::string to_expression_string(T const* expr_node_ptr)
{
    throw std::logic_error("to_expression_string() called with pointer argument");
    // compile error intended here; comment on the next line shows in clang output
    return expr_node_ptr; // to_expression_string() called with pointer argument
}

template <typename T>
std::string to_expression_string(boost::shared_ptr<T> const& expr_node_ptr)
{
    throw std::logic_error("to_expression_string() called with pointer argument");
    // compile error intended here; comment on the next line shows in clang output
    return expr_node_ptr; // to_expression_string() called with pointer argument
}
}

#endif // MAPNIK_EXPRESSION_STRING_HPP
