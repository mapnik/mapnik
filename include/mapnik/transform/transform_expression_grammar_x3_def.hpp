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

#ifndef MAPNIK_TRANSFORM_GRAMMAR_X3_DEF_HPP
#define MAPNIK_TRANSFORM_GRAMMAR_X3_DEF_HPP

#include <mapnik/transform/transform_expression.hpp>
#include <mapnik/transform/transform_expression_grammar_x3.hpp>
#include <mapnik/expression_grammar_x3.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#pragma GCC diagnostic pop

// skewX
BOOST_FUSION_ADAPT_STRUCT(mapnik::skewX_node,
                          (mapnik::expr_node, angle_))

// skewY
BOOST_FUSION_ADAPT_STRUCT(mapnik::skewY_node,
                          (mapnik::expr_node, angle_))


// Following specializations are required to avoid trasferring mapnik::skewX/Y nodes as underlying expressions
//
// template <typename Source, typename Dest>
// inline void
// move_to_plain(Source&& src, Dest& dest, mpl::true_) // src is a single-element tuple
// {
//     dest = std::move(fusion::front(src));
// }
// which will fail to compile with latest (more strict) `mapbox::variant` (boost_1_61)

namespace boost { namespace spirit { namespace x3 { namespace traits {
template <>
inline void move_to<mapnik::skewX_node, mapnik::detail::transform_node>(mapnik::skewX_node && src, mapnik::detail::transform_node& dst)
{
    dst = std::move(src);
}

template <>
inline void move_to<mapnik::skewY_node, mapnik::detail::transform_node>(mapnik::skewY_node && src, mapnik::detail::transform_node& dst)
{
    dst = std::move(src);
}

}}}}


namespace mapnik { namespace grammar {

namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;

// [http://www.w3.org/TR/SVG/coords.html#TransformAttribute]

// The value of the ‘transform’ attribute is a <transform-list>, which
// is defined as a list of transform definitions, which are applied in
// the order provided.  The individual transform definitions are
// separated by whitespace and/or a comma.

using x3::double_;
using x3::no_skip;
using x3::no_case;
using x3::lit;

auto const create_expr_node = [](auto const& ctx)
{
    _val(ctx) = _attr(ctx);
};

auto const construct_matrix = [] (auto const& ctx)
{
    auto const& attr = _attr(ctx);
    auto const& a = boost::fusion::at<boost::mpl::int_<0>>(attr);
    auto const& b = boost::fusion::at<boost::mpl::int_<1>>(attr);
    auto const& c = boost::fusion::at<boost::mpl::int_<2>>(attr);
    auto const& d = boost::fusion::at<boost::mpl::int_<3>>(attr);
    auto const& e = boost::fusion::at<boost::mpl::int_<4>>(attr);
    auto const& f = boost::fusion::at<boost::mpl::int_<5>>(attr);
    _val(ctx) = mapnik::matrix_node(a, b, c, d, e, f);
};

auto const construct_translate = [](auto const& ctx)
{
    auto const& attr = _attr(ctx);
    auto const& dx = boost::fusion::at<boost::mpl::int_<0>>(attr);
    auto const& dy = boost::fusion::at<boost::mpl::int_<1>>(attr); //optional
    _val(ctx) = mapnik::translate_node(dx, dy);
};

auto const construct_scale = [](auto const& ctx)
{
    auto const& attr = _attr(ctx);
    auto const& sx = boost::fusion::at<boost::mpl::int_<0>>(attr);
    auto const& sy = boost::fusion::at<boost::mpl::int_<1>>(attr); //optional
    _val(ctx) = mapnik::scale_node(sx, sy);
};

auto const construct_rotate = [](auto const& ctx)
{
    auto const& attr = _attr(ctx);
    auto const& a = boost::fusion::at<boost::mpl::int_<0>>(attr);
    auto const& sx = boost::fusion::at<boost::mpl::int_<1>>(attr); //optional
    auto const& sy = boost::fusion::at<boost::mpl::int_<2>>(attr); //optional
    _val(ctx) = mapnik::rotate_node(a, sx, sy);
};

// rules
x3::rule<class transform_list_class, mapnik::transform_list> const transform_list_rule("transform list");
x3::rule<class transform_node_class, mapnik::transform_node> const transform_node_rule("transform node");
x3::rule<class matrix_node_class, mapnik::matrix_node> const matrix("matrix node");
x3::rule<class translate_node_class, mapnik::translate_node> const translate("translate node");
x3::rule<class scale_node_class, mapnik::scale_node> const scale("scale node");
x3::rule<class rotate_node_class, mapnik::rotate_node> const rotate("rotate node");
x3::rule<class skewX_node_class, mapnik::skewX_node> const skewX("skew X node");
x3::rule<class skewY_node_class, mapnik::skewY_node> const skewY("skew Y node");
x3::rule<class expr_tag, mapnik::expr_node> const expr("Expression");
x3::rule<class sep_expr_tag, mapnik::expr_node> const sep_expr("Separated Expression");

// start
auto const transform_def = transform_list_rule;

auto const transform_list_rule_def = transform_node_rule % *lit(',');

auto const transform_node_rule_def = matrix | translate  | scale | rotate | skewX | skewY ;

// number or attribute
auto const atom = x3::rule<class atom_tag, expr_node> {} = double_[create_expr_node]
    ;

auto const sep_atom = x3::rule<class sep_atom_tag, expr_node> {} = -lit(',') >> double_[create_expr_node]
    ;

auto const expr_def = expression;
// Individual arguments in lists containing one or more compound
// expressions are separated by a comma.
auto const sep_expr_def = lit(',') > expr
    ;

// matrix(<a> <b> <c> <d> <e> <f>)
auto const matrix_def = no_case[lit("matrix")] > '('
    > ((atom >> sep_atom >> sep_atom >> sep_atom >> sep_atom >> sep_atom >> lit(')'))[construct_matrix]
       |
       (expr > sep_expr > sep_expr > sep_expr > sep_expr > sep_expr > lit(')'))[construct_matrix])
    ;

// translate(<tx> [<ty>])
auto const translate_def = no_case[lit("translate")] > lit('(')
    > (( atom >> -sep_atom  >> lit(')'))[construct_translate]
       |
       ( expr > -sep_expr > lit(')'))[construct_translate]
        );

// scale(<sx> [<sy>])
auto const scale_def = no_case[lit("scale")] > lit('(')
    > (( atom >> -sep_atom  >> lit(')'))[construct_scale]
       |
       ( expr > -sep_expr > lit(')'))[construct_scale]
        );

// rotate(<rotate-angle> [<cx> <cy>])
auto const rotate_def = no_case[lit("rotate")] > lit('(')
    > ((atom >> -sep_atom >> -sep_atom >>  lit(')'))[construct_rotate]
       |
       (expr > -sep_expr > -sep_expr > lit(')'))[construct_rotate]
        );

// skewX(<skew-angle>)
auto const skewX_def = no_case[lit("skewX")]
    > '(' > expr > ')';

// skewY(<skew-angle>)
auto const skewY_def = no_case[lit("skewY")]
    > '(' >  expr  > ')';

BOOST_SPIRIT_DEFINE (
    expr,
    sep_expr,
    transform,
    transform_list_rule,
    transform_node_rule,
    matrix,
    translate,
    scale,
    rotate,
    skewX,
    skewY);

}} // ns

#endif
