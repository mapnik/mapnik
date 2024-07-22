/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

// mapnik

#include <mapnik/config.hpp>            // needed by msvc
#include <mapnik/expression_string.hpp> // needed by msvc
#include <mapnik/expression_node_types.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/value.hpp>

namespace mapnik {

struct expression_string
{
    explicit expression_string(std::string& str)
        : str_(str)
    {}

    void operator()(value_type const& x) const { str_ += x.to_expression_string(); }

    void operator()(attribute const& attr) const
    {
        str_ += "[";
        str_ += attr.name();
        str_ += "]";
    }

    void operator()(global_attribute const& attr) const
    {
        str_ += "@";
        str_ += attr.name;
    }

    void operator()(geometry_type_attribute const& /*attr*/) const { str_ += "[mapnik::geometry_type]"; }

    template<typename Tag>
    void operator()(binary_node<Tag> const& x) const
    {
        if (x.type() != tags::mult::str() && x.type() != tags::div::str())
        {
            str_ += "(";
        }

        util::apply_visitor(*this, x.left);
        str_ += x.type();
        util::apply_visitor(*this, x.right);
        if (x.type() != tags::mult::str() && x.type() != tags::div::str())
        {
            str_ += ")";
        }
    }

    template<typename Tag>
    void operator()(unary_node<Tag> const& x) const
    {
        str_ += Tag::str();
        str_ += "(";
        util::apply_visitor(*this, x.expr);
        str_ += ")";
    }

    void operator()(regex_match_node const& x) const
    {
        util::apply_visitor(*this, x.expr);
        str_ += x.to_string();
    }

    void operator()(regex_replace_node const& x) const
    {
        util::apply_visitor(*this, x.expr);
        str_ += x.to_string();
    }

    void operator()(unary_function_call const& call) const
    {
        str_ += unary_function_name(call.fun);
        str_ += "(";
        util::apply_visitor(*this, call.arg);
        str_ += ")";
    }
    void operator()(binary_function_call const& call) const
    {
        str_ += binary_function_name(call.fun);
        str_ += "(";
        util::apply_visitor(*this, call.arg1);
        str_ += ",";
        util::apply_visitor(*this, call.arg2);
        str_ += ")";
    }

  private:
    std::string& str_;
};

std::string to_expression_string(expr_node const& node)
{
    std::string str;
    expression_string functor(str);
    util::apply_visitor(std::ref(functor), node);
    return str;
}

} // namespace mapnik
