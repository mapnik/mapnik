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

#ifndef MAPNIK_EXPRESSION_NODE_TYPES_HPP
#define MAPNIK_EXPRESSION_NODE_TYPES_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/util/variant.hpp>

namespace mapnik
{

namespace tags  {
struct negate
{
    static const char* str()
    {
        return "-";
    }
};

struct plus
{
    static const char* str()
    {
        return "+";
    }
};

struct minus
{
    static const char* str()
    {
        return "-";
    }
};

struct mult
{
    static const char* str()
    {
        return "*";
    }
};

struct div
{
    static const char* str()
    {
        return "/";
    }
};


struct  mod
{
    static const char* str()
    {
        return "%";
    }
};

struct less
{
    static const char* str()
    {
        return "<";
    }
};

struct less_equal
{
    static const char* str()
    {
        return "<=";
    }
};

struct greater
{
    static const char* str()
    {
        return ">";
    }
};

struct greater_equal
{
    static const char* str()
    {
        return ">=";
    }
};

struct equal_to
{
    static const char* str()
    {
        return "=";
    }
};

struct not_equal_to
{
    static const char* str()
    {
        return "!=";
    }
};

struct logical_not
{
    static const char* str()
    {
        return "not ";
    }
};

struct logical_and
{
    static const char* str()
    {
        return " and ";
    }
};

struct logical_or
{
    static const char* str()
    {
        return " or ";
    }
};

} // end operation tags


template <typename Tag> struct binary_node;
template <typename Tag> struct unary_node;
struct regex_match_node;
struct regex_replace_node;
struct attribute;
struct global_attribute;
struct geometry_type_attribute;
struct unary_function_call;
struct binary_function_call;

using expr_node  = util::variant<
value_null,
value_bool,
value_integer,
value_double,
value_unicode_string,
attribute,
global_attribute,
geometry_type_attribute,
util::recursive_wrapper<unary_node<tags::negate> >,
util::recursive_wrapper<binary_node<tags::plus> >,
util::recursive_wrapper<binary_node<tags::minus> >,
util::recursive_wrapper<binary_node<tags::mult> >,
util::recursive_wrapper<binary_node<tags::div> >,
util::recursive_wrapper<binary_node<tags::mod> >,
util::recursive_wrapper<binary_node<tags::less> >,
util::recursive_wrapper<binary_node<tags::less_equal> >,
util::recursive_wrapper<binary_node<tags::greater> >,
util::recursive_wrapper<binary_node<tags::greater_equal> >,
util::recursive_wrapper<binary_node<tags::equal_to> >,
util::recursive_wrapper<binary_node<tags::not_equal_to> >,
util::recursive_wrapper<unary_node<tags::logical_not> >,
util::recursive_wrapper<binary_node<tags::logical_and> >,
util::recursive_wrapper<binary_node<tags::logical_or> >,
util::recursive_wrapper<regex_match_node>,
util::recursive_wrapper<regex_replace_node>,
util::recursive_wrapper<unary_function_call>,
util::recursive_wrapper<binary_function_call>
>;

}


#endif // MAPNIK_EXPRESSION_NODE_HPP
