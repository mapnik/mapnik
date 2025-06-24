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

namespace mapnik {

template<typename T0, typename T1, typename T2>
struct evaluate
{
    using feature_type = T0;
    using value_type = T1;
    using variable_type = T2;
    using result_type = T1; //  we need this because automatic result_type deduction fails
    explicit evaluate(feature_type const& f, variable_type const& v)
        : feature_(f)
        , vars_(v)
    {}

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
        return (util::apply_visitor(*this, x.left).to_bool()) && (util::apply_visitor(*this, x.right).to_bool());
    }

    value_type operator()(binary_node<tags::logical_or> const& x) const
    {
        return (util::apply_visitor(*this, x.left).to_bool()) || (util::apply_visitor(*this, x.right).to_bool());
    }

    template<typename Tag>
    value_type operator()(binary_node<Tag> const& x) const
    {
        typename make_op<Tag>::type operation;
        return operation(util::apply_visitor(*this, x.left), util::apply_visitor(*this, x.right));
    }

    template<typename Tag>
    value_type operator()(unary_node<Tag> const& x) const
    {
        typename make_op<Tag>::type func;
        return func(util::apply_visitor(*this, x.expr));
    }

    value_type operator()(unary_node<tags::logical_not> const& x) const
    {
        return !(util::apply_visitor(*this, x.expr).to_bool());
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
