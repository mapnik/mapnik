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

// Dummy types that are used to trigger nice (with gcc at least)
// compilation error when to_expression_string is misused.
enum expr_node_ref_ {};
enum expr_node_ptr_ {};

// The following two templates should prevent accidentally passing
// a pointer (either raw or shared) as the argument.  Without them,
// the compiler would construct a temporary expr_node(bool) using
// implicit pointer-to-bool conversion, thus any non-null pointer
// would yield "true".

template <typename T>
std::string to_expression_string(T const* x)
{
    expr_node_ref_ invalid_argument_type = expr_node_ptr_();
    throw std::logic_error("to_expression_string() called with pointer argument");
    return std::string();
}

template <typename T>
std::string to_expression_string(boost::shared_ptr<T> const& x)
{
    expr_node_ref_ invalid_argument_type = expr_node_ptr_();
    throw std::logic_error("to_expression_string() called with pointer argument");
    return std::string();
}
}

#endif // MAPNIK_EXPRESSION_STRING_HPP
