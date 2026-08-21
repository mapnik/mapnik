/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_EXPRESSION_EVALUATOR_HPP
#define MAPNIK_EXPRESSION_EVALUATOR_HPP

// mapnik
#include <mapnik/attribute.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/function_call.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/feature.hpp>

// stl
#include <type_traits>

namespace mapnik {

namespace detail {

template<typename Tag>
struct ustring_compare
{
    static constexpr bool enabled = false;
};

template<>
struct ustring_compare<mapnik::tags::equal_to>
{
    static constexpr bool enabled = true;

    static bool apply(value_unicode_string const& a, value_unicode_string const& b) { return a == b; }

    template<typename V>
    static bool apply_value(V const& lhs, value_unicode_string const& rhs)
    {
        if (lhs.template is<value_unicode_string>())
        {
            return lhs.template get_unchecked<value_unicode_string>() == rhs;
        }
        return false;
    }
};

template<>
struct ustring_compare<mapnik::tags::not_equal_to>
{
    static constexpr bool enabled = true;

    static bool apply(value_unicode_string const& a, value_unicode_string const& b) { return a != b; }

    // Preserve value's asymmetric null/empty-string comparison.
    template<typename V>
    static bool apply_value(V const& lhs, value_unicode_string const& rhs)
    {
        if (lhs.template is<value_unicode_string>())
        {
            return lhs.template get_unchecked<value_unicode_string>() != rhs;
        }
        if (lhs.template is<value_null>())
        {
            return !rhs.isEmpty();
        }
        return true;
    }
};

} // namespace detail

template<typename T0, typename T1, typename T2>
struct evaluate
{
    using feature_type = T0;
    using value_type = T1;
    using variable_type = T2;
    using result_type = T1; //  we need this because automatic result_type deduction fails
    explicit evaluate(feature_type const& f, variable_type const& v)
        : feature_(f),
          vars_(v)
    {}

    using attribute_reference =
      decltype(std::declval<feature_type const&>().get(std::declval<std::string const&>()));
    static constexpr bool borrowable_attributes =
      std::is_lvalue_reference_v<attribute_reference> && std::is_same_v<std::decay_t<attribute_reference>, value_type>;

    value_type const* borrow(expr_node const& node) const
    {
        if constexpr (borrowable_attributes)
        {
            if (node.template is<attribute>())
            {
                return &feature_.get(node.template get_unchecked<attribute>().name());
            }
        }
        return nullptr;
    }

    value_bool eval_to_bool(expr_node const& node) const
    {
        if (value_type const* v = borrow(node))
        {
            return v->to_bool();
        }
        return util::apply_visitor(*this, node).to_bool();
    }

    value_type operator()(value_integer val) const { return val; }

    value_type operator()(value_double val) const { return val; }

    value_type operator()(value_bool val) const { return val; }

    value_type operator()(value_null val) const { return val; }

    value_type operator()(value_unicode_string const& str) const { return str; }

    value_type operator()(attribute const& attr) const { return attr.value<value_type, feature_type>(feature_); }

    value_type operator()(global_attribute const& attr) const
    {
        auto itr = vars_.find(attr.name);
        if (itr != vars_.end())
        {
            return itr->second;
        }
        return value_type(); // throw?
    }

    value_type operator()(geometry_type_attribute const& geom) const
    {
        return geom.value<value_type, feature_type>(feature_);
    }

    value_type operator()(binary_node<tags::logical_and> const& x) const
    {
        return eval_to_bool(x.left) && eval_to_bool(x.right);
    }

    value_type operator()(binary_node<tags::logical_or> const& x) const
    {
        return eval_to_bool(x.left) || eval_to_bool(x.right);
    }

    template<typename Tag>
    value_type operator()(binary_node<Tag> const& x) const
    {
        typename make_op<Tag>::type operation;
        value_type const* lhs = borrow(x.left);
        value_type const* rhs = borrow(x.right);

        if constexpr (detail::ustring_compare<Tag>::enabled)
        {
            if (lhs && !rhs && x.right.template is<value_unicode_string>())
            {
                return value_type(detail::ustring_compare<Tag>::apply_value(
                  *lhs,
                  x.right.template get_unchecked<value_unicode_string>()));
            }
            if (rhs && !lhs && rhs->template is<value_unicode_string>() &&
                x.left.template is<value_unicode_string>())
            {
                return value_type(
                  detail::ustring_compare<Tag>::apply(x.left.template get_unchecked<value_unicode_string>(),
                                                      rhs->template get_unchecked<value_unicode_string>()));
            }
        }

        if (lhs)
        {
            if (rhs)
                return operation(*lhs, *rhs);
            return operation(*lhs, util::apply_visitor(*this, x.right));
        }
        if (rhs)
            return operation(util::apply_visitor(*this, x.left), *rhs);
        return operation(util::apply_visitor(*this, x.left), util::apply_visitor(*this, x.right));
    }

    template<typename Tag>
    value_type operator()(unary_node<Tag> const& x) const
    {
        typename make_op<Tag>::type func;
        if (value_type const* v = borrow(x.expr))
        {
            return func(*v);
        }
        return func(util::apply_visitor(*this, x.expr));
    }

    value_type operator()(unary_node<tags::logical_not> const& x) const
    {
        return !eval_to_bool(x.expr);
    }

    value_type operator()(regex_match_node const& x) const
    {
        value_type v = util::apply_visitor(*this, x.expr);
        return x.apply(v);
    }

    value_type operator()(regex_replace_node const& x) const
    {
        value_type v = util::apply_visitor(*this, x.expr);
        return x.apply(v);
    }

    value_type operator()(unary_function_call const& call) const
    {
        value_type arg = util::apply_visitor(*this, call.arg);
        return call.fun(arg);
    }

    value_type operator()(binary_function_call const& call) const
    {
        value_type arg1 = util::apply_visitor(*this, call.arg1);
        value_type arg2 = util::apply_visitor(*this, call.arg2);
        return call.fun(arg1, arg2);
    }

    feature_type const& feature_;
    variable_type const& vars_;
};

} // namespace mapnik

#endif // MAPNIK_EXPRESSION_EVALUATOR_HPP
