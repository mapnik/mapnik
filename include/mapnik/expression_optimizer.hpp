/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

#ifndef MAPNIK_EXPRESSION_OPTIMIZER_HPP
#define MAPNIK_EXPRESSION_OPTIMIZER_HPP

// mapnik
#include <mapnik/attribute.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/expression_node.hpp>

// boost
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>
#include <tuple>

namespace mapnik
{


template <typename T>
struct optimize : boost::static_visitor< std::tuple<T,bool> >
{
    typedef T node_type;
    typedef std::tuple<node_type,bool> return_type;

    return_type operator() (value const& v) const
    {
        return std::make_tuple(node_type(v),true);
    }

    return_type operator() (attribute const& attr) const
    {
        return std::make_tuple(node_type(attr),false);
    }

    return_type operator() (geometry_type_attribute const& attr) const
    {
        return return_type(node_type(attr),false);
    }

    return_type operator() (binary_node<tags::logical_and> const & x) const
    {
        return return_type(node_type(x),false);
//        return (boost::apply_visitor(optimize<node_type>(),x.left).to_bool())
        //          && (boost::apply_visitor(optimize<node_type>(),x.right).to_bool());
    }

    return_type operator() (binary_node<tags::logical_or> const & x) const
    {
        return return_type(node_type(x),false);
//        return (boost::apply_visitor(optimize<node_type>(),x.left).to_bool())
        //          || (boost::apply_visitor(optimize<node_type>(),x.right).to_bool());
    }

    template <typename Tag>
    return_type operator() (binary_node<Tag> const& x) const
    {
        auto left  = boost::apply_visitor(optimize<node_type>(),x.left);
        auto right = boost::apply_visitor(optimize<node_type>(),x.right);
        if (std::get<1>(left) && std::get<1>(right))
        {
            // evaluate
            typename make_op<Tag>::type operation;
            auto lv = boost::get<mapnik::value_type>(std::get<0>(left));
            auto rv = boost::get<mapnik::value_type>(std::get<0>(right));
            return return_type(node_type(operation(lv,rv)),true);
        }

        if (std::get<1>(right))
        {
            return return_type(node_type(binary_node<Tag>(std::get<0>(right),
                                                          std::get<0>(left))), false);
        }
        if (std::get<1>(left))
        {
            return return_type(node_type(binary_node<Tag>(std::get<0>(left),
                                                          std::get<0>(right))), false);
        }
        return return_type(node_type(x),false);
    }

    template <typename Tag>
    return_type operator() (unary_node<Tag> const& x) const
    {
        //typename make_op<Tag>::type func;
        //return func(boost::apply_visitor(*this, x.expr));
//        return node_type(x);
        return return_type(node_type(x),false);
    }

    return_type operator() (unary_node<tags::logical_not> const& x) const
    {
//        return node_type(x);
        return return_type(node_type(x),false);
        //return ! (boost::apply_visitor(optimize<node_type>(),x.expr).to_bool());
    }

    return_type operator() (regex_match_node const& x) const
    {
        return return_type(node_type(x),false);
//        return boost::apply_visitor(optimize<node_type>(),x.expr);
    }

    return_type operator() (regex_replace_node const& x) const
    {
        return return_type(node_type(x),false);
        //return boost::apply_visitor(optimize<node_type>(),x.expr);
    }
};

}

#endif // MAPNIK_EXPRESSION_OPTIMIZER_HPP
