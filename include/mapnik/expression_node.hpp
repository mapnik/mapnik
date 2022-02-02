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

#ifndef MAPNIK_EXPRESSION_NODE_HPP
#define MAPNIK_EXPRESSION_NODE_HPP

// mapnik
#include <mapnik/value/types.hpp>
#include <mapnik/value.hpp>
#include <mapnik/config.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/function_call.hpp>
#include <mapnik/expression_node_types.hpp>
// stl
#include <memory>

namespace mapnik {

using value_type = mapnik::value;

template<typename Tag>
struct make_op;
template<>
struct make_op<mapnik::tags::negate>
{
    using type = std::negate<value_type>;
};
template<>
struct make_op<mapnik::tags::plus>
{
    using type = std::plus<value_type>;
};
template<>
struct make_op<mapnik::tags::minus>
{
    using type = std::minus<value_type>;
};
template<>
struct make_op<mapnik::tags::mult>
{
    using type = std::multiplies<value_type>;
};
template<>
struct make_op<mapnik::tags::div>
{
    using type = std::divides<value_type>;
};
template<>
struct make_op<mapnik::tags::mod>
{
    using type = std::modulus<value_type>;
};
template<>
struct make_op<mapnik::tags::less>
{
    using type = std::less<value_type>;
};
template<>
struct make_op<mapnik::tags::less_equal>
{
    using type = std::less_equal<value_type>;
};
template<>
struct make_op<mapnik::tags::greater>
{
    using type = std::greater<value_type>;
};
template<>
struct make_op<mapnik::tags::greater_equal>
{
    using type = std::greater_equal<value_type>;
};
template<>
struct make_op<mapnik::tags::equal_to>
{
    using type = std::equal_to<value_type>;
};
template<>
struct make_op<mapnik::tags::not_equal_to>
{
    using type = std::not_equal_to<value_type>;
};
template<>
struct make_op<mapnik::tags::logical_not>
{
    using type = std::logical_not<value_type>;
};
template<>
struct make_op<mapnik::tags::logical_and>
{
    using type = std::logical_and<value_type>;
};
template<>
struct make_op<mapnik::tags::logical_or>
{
    using type = std::logical_or<value_type>;
};

template<typename Tag>
struct unary_node
{
    unary_node(expr_node&& a)
        : expr(std::move(a))
    {}

    unary_node(expr_node const& a)
        : expr(a)
    {}

    static const char* type() { return Tag::str(); }

    expr_node expr;
};

template<typename Tag>
struct binary_node
{
    binary_node(expr_node&& a, expr_node&& b)
        : left(std::move(a))
        , right(std::move(b))
    {}

    binary_node(expr_node const& a, expr_node const& b)
        : left(a)
        , right(b)
    {}

    static const char* type() { return Tag::str(); }
    expr_node left, right;
};

struct unary_function_call
{
    using argument_type = expr_node;
    unary_function_call() = default;
    unary_function_call(unary_function_impl _fun, argument_type const& _arg)
        : fun(_fun)
        , arg(_arg)
    {}

    unary_function_impl fun;
    argument_type arg;
};

struct binary_function_call
{
    using argument_type = expr_node;
    binary_function_call() = default;
    binary_function_call(binary_function_impl _fun, argument_type const& _arg1, argument_type const& _arg2)
        : fun(_fun)
        , arg1(_arg1)
        , arg2(_arg2)
    {}
    binary_function_impl fun;
    argument_type arg1;
    argument_type arg2;
};

// pimpl
struct _regex_match_impl;
struct _regex_replace_impl;

struct MAPNIK_DECL regex_match_node
{
    regex_match_node(transcoder const& tr, expr_node const& a, std::string const& ustr);
    mapnik::value apply(mapnik::value const& v) const;
    std::string to_string() const;
    expr_node expr;
    // TODO - use unique_ptr once https://github.com/mapnik/mapnik/issues/2457 is fixed
    std::shared_ptr<_regex_match_impl> impl_;
};

struct MAPNIK_DECL regex_replace_node
{
    regex_replace_node(transcoder const& tr, expr_node const& a, std::string const& ustr, std::string const& f);
    mapnik::value apply(mapnik::value const& v) const;
    std::string to_string() const;
    expr_node expr;
    // TODO - use unique_ptr once https://github.com/mapnik/mapnik/issues/2457 is fixed
    std::shared_ptr<_regex_replace_impl> impl_;
};

} // namespace mapnik

#endif // MAPNIK_EXPRESSION_NODE_HPP
