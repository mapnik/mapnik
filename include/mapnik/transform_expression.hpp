/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_TRANSFORM_EXPRESSION_HPP
#define MAPNIK_TRANSFORM_EXPRESSION_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/expression_node_types.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/util/variant.hpp>
// boost
#include <boost/optional.hpp>
// fusion
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/vector.hpp>

// stl
#include <vector>
#include <memory>

namespace mapnik {

struct identity_node {};

struct matrix_node
{
    expr_node a_;
    expr_node b_;
    expr_node c_;
    expr_node d_;
    expr_node e_;
    expr_node f_;

    template <typename T>
    explicit matrix_node(T const& m)
        : a_(m.sx), b_(m.shy), c_(m.shx), d_(m.sy), e_(m.tx), f_(m.ty) {}

    matrix_node(expr_node const& a, expr_node const& b, expr_node const& c,
                expr_node const& d, expr_node const& e, expr_node const& f)
        : a_(a), b_(b), c_(c), d_(d), e_(e), f_(f) {}
};

struct translate_node
{
    expr_node tx_;
    expr_node ty_;

    translate_node(expr_node const& tx,
                   boost::optional<expr_node> const& ty)
        : tx_(tx)
        , ty_(ty ? expr_node(*ty) : value_null()) {}
};

struct scale_node
{
    expr_node sx_;
    expr_node sy_;

    scale_node(expr_node const& sx,
               boost::optional<expr_node> const& sy)
        : sx_(sx)
        , sy_(sy ? expr_node(*sy) : value_null()) {}
};

struct rotate_node
{
    using coords_type = boost::fusion::vector2<expr_node, expr_node>;

    expr_node angle_;
    expr_node cx_;
    expr_node cy_;

    explicit rotate_node(expr_node const& angle)
        : angle_(angle) {}

    rotate_node(expr_node const& angle,
                expr_node const& cx, expr_node const& cy)
        : angle_(angle), cx_(cx), cy_(cy) {}

    rotate_node(expr_node const& angle,
                boost::optional<expr_node> const& cx,
                boost::optional<expr_node> const& cy)
        : angle_(angle)
        , cx_(cx ? expr_node(*cx) : value_null())
        , cy_(cy ? expr_node(*cy) : value_null()) {}

    rotate_node(expr_node const& angle,
                boost::optional<coords_type> const& center)
      : angle_(angle)
    {
        if (center)
        {
            cx_ = boost::fusion::at_c<0>(*center);
            cy_ = boost::fusion::at_c<1>(*center);
        }
    }
};

struct skewX_node
{
    expr_node angle_;

    explicit skewX_node(expr_node const& angle)
        : angle_(angle) {}
};

struct skewY_node
{
    expr_node angle_;

    explicit skewY_node(expr_node const& angle)
        : angle_(angle) {}
};

namespace detail {

// boost::spirit::traits::clear<T>(T& val) [with T = boost::variant<...>]
// attempts to assign to the variant's current value a default-constructed
// value of the same type, which not only requires that each value-type is
// default-constructible, but also makes little sense with our variant of
// transform nodes...

using transform_variant =  mapnik::util::variant< identity_node,
                                                  matrix_node,
                                                  translate_node,
                                                  scale_node,
                                                  rotate_node,
                                                  skewX_node,
                                                  skewY_node >;

// ... thus we wrap the variant-type in a distinct type and provide
// a custom clear overload, which resets the value to identity_node

struct transform_node
{
    transform_variant base_;

    transform_node()
        : base_() {}

    template <typename T>
    transform_node(T const& val)
        : base_(val) {}

    template <typename T>
    transform_node& operator= (T const& val)
    {
        base_ = val;
        return *this;
    }

    transform_variant const& operator* () const
    {
        return base_;
    }

    transform_variant& operator* ()
    {
        return base_;
    }
};

inline void clear(transform_node& val)
{
    val.base_ = identity_node();
}

namespace  {

struct is_null_transform_node
{
    bool operator() (value const& val) const
    {
        return val.is_null();
    }

    bool operator() (value_null const&) const
    {
        return true;
    }

    template <typename T>
    bool operator() (T const&) const
    {
        return false;
    }

    bool operator() (detail::transform_variant const& var) const
    {
        return util::apply_visitor(*this, var);
    }
};

}

template <typename T>
bool is_null_node (T const& node)
{
    return util::apply_visitor(is_null_transform_node(), node);
}

} // namespace detail

using transform_node = detail::transform_node             ;
using transform_list = std::vector<transform_node>        ;
using transform_list_ptr = std::shared_ptr<transform_list>  ;

MAPNIK_DECL std::string to_expression_string(transform_node const& node);
MAPNIK_DECL std::string to_expression_string(transform_list const& list);

} // namespace mapnik

#endif // MAPNIK_TRANSFORM_EXPRESSION_HPP
