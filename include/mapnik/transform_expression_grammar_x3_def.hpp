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

#ifndef MAPNIK_TRANSFORM_GRAMMAR_X3_DEF_HPP
#define MAPNIK_TRANSFORM_GRAMMAR_X3_DEF_HPP

#include <mapnik/transform_expression.hpp>
#include <mapnik/transform_expression_grammar_x3.hpp>
#include <mapnik/expression_grammar_x3.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#pragma GCC diagnostic pop

// adapt transform nodes
// martrix
BOOST_FUSION_ADAPT_STRUCT(mapnik::matrix_node,
                          (mapnik::expr_node, a_)
                          (mapnik::expr_node, b_)
                          (mapnik::expr_node, c_)
                          (mapnik::expr_node, d_)
                          (mapnik::expr_node, e_)
                          (mapnik::expr_node, f_))
// translate
BOOST_FUSION_ADAPT_STRUCT(mapnik::translate_node,
                          (mapnik::expr_node, tx_)
                          (mapnik::expr_node, ty_))

// scale
BOOST_FUSION_ADAPT_STRUCT(mapnik::scale_node,
                          (mapnik::expr_node, sx_)
                          (mapnik::expr_node, sy_))

// rotate
BOOST_FUSION_ADAPT_STRUCT(mapnik::rotate_node,
                          (mapnik::expr_node, angle_)
                          (mapnik::expr_node, cx_)
                          (mapnik::expr_node, cy_))
// skewX
BOOST_FUSION_ADAPT_STRUCT(mapnik::skewX_node,
                          (mapnik::expr_node, angle_))

// skewY
BOOST_FUSION_ADAPT_STRUCT(mapnik::skewY_node,
                          (mapnik::expr_node, angle_))


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
    using x3::char_;

    // starting rule
    transform_expression_grammar_type const transform("transform");
    // rules
    x3::rule<class transform_list_class, mapnik::transform_list> transform_list_rule("transform list");
    x3::rule<class transform_node_class, mapnik::transform_node> transform_node_rule("transform node");
    x3::rule<class matrix_node_class, mapnik::matrix_node> matrix("matrix node");
    x3::rule<class translate_node_class, mapnik::translate_node> translate("translate node");
    x3::rule<class scale_node_class, mapnik::scale_node> scale("scale node");
    x3::rule<class rotate_node_class, mapnik::rotate_node> rotate("rotate node");
    x3::rule<class skewX_node_class, mapnik::skewX_node> skewX("skew X node");
    x3::rule<class skewY_node_class, mapnik::skewY_node> skewY("skew Y node");

    // Import the expression rule
    namespace { auto const& expr = expression_grammar(); }

    // start
    auto const transform_def = transform_list_rule;

    auto const transform_list_rule_def = transform_node_rule % no_skip[*char_(", ")];

    auto const transform_node_rule_def = matrix | translate  | scale | rotate | skewX | skewY ;

    // matrix(<a> <b> <c> <d> <e> <f>)
    auto const matrix_def = no_case[lit("matrix")]
        > '(' > expr > ',' >  expr > ',' > expr > ',' > expr > ',' > expr > ',' > expr > ')'
        ;

    // translate(<tx> [<ty>])
    auto const translate_def = no_case[lit("translate")]
        > '(' > expr > -(',' > expr )  > ')'
        ;

    // scale(<sx> [<sy>])
    auto const scale_def = no_case[lit("scale")]
        > '(' > expr >  -(',' > expr )  > ')'
        ;

    // rotate(<rotate-angle> [<cx> <cy>])
    auto const rotate_def = no_case[lit("rotate")]
        > '(' > expr > -(',' > expr > ',' > expr) > ')'
        ;

    // skewX(<skew-angle>)
    auto const skewX_def = no_case[lit("skewX")]
        > '(' > expr > ')';

    // skewY(<skew-angle>)
    auto const skewY_def = no_case[lit("skewY")]
        > '(' >  expr  > ')';


    BOOST_SPIRIT_DEFINE (
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

namespace mapnik
{
grammar::transform_expression_grammar_type transform_expression_grammar()
{
    return grammar::transform;
}
}

#endif
