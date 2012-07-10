/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#ifndef MAPNIK_TRANSFORM_EXPRESSION_GRAMMAR_HPP
#define MAPNIK_TRANSFORM_EXPRESSION_GRAMMAR_HPP

// mapnik
#include <mapnik/expression_grammar.hpp>
#include <mapnik/transform_expression.hpp>

// spirit
#include <boost/spirit/home/phoenix/object/construct.hpp>
#include <boost/spirit/home/qi.hpp>

namespace mapnik {

    namespace qi = boost::spirit::qi;

    template <typename Iterator>
    struct transform_expression_grammar
        : qi::grammar<Iterator, transform_list(), space_type>
    {
        explicit transform_expression_grammar(expression_grammar<Iterator> const& g)
            : transform_expression_grammar::base_type(start)
        {
            using boost::phoenix::construct;
            using qi::_a; using qi::_1; using qi::_4;
            using qi::_b; using qi::_2; using qi::_5;
            using qi::_c; using qi::_3; using qi::_6;
            using qi::_val;
            using qi::char_;
            using qi::lit;
            using qi::no_case;
            using qi::no_skip;

            start = transform_ % no_skip[char_(", ")] ;

            transform_ = matrix | translate | scale | rotate | skewX | skewY ;

            matrix = no_case[lit("matrix")]
                >> (lit('(')
                    >> expr >> -lit(',')
                    >> expr >> -lit(',')
                    >> expr >> -lit(',')
                    >> expr >> -lit(',')
                    >> expr >> -lit(',')
                    >> expr >>  lit(')'))
                [ _val = construct<matrix_node>(_1,_2,_3,_4,_5,_6) ];

            translate = no_case[lit("translate")]
                >> (lit('(')
                    >> expr >> -lit(',')
                    >> -expr >> lit(')'))
                [ _val = construct<translate_node>(_1,_2) ];

            scale = no_case[lit("scale")]
                >> (lit('(')
                    >> expr >> -lit(',')
                    >> -expr >> lit(')'))
                [ _val = construct<scale_node>(_1,_2) ];

            rotate = no_case[lit("rotate")]
                >> lit('(')
                >> expr[_a = _1] >> -lit(',')
                >> -(expr [_b = _1] >> -lit(',') >> expr[_c = _1])
                >> lit(')')
                [ _val = construct<rotate_node>(_a,_b,_c) ];

            skewX = no_case[lit("skewX")]
                >> lit('(')
                >> expr [ _val = construct<skewX_node>(_1) ]
                >> lit(')');

            skewY = no_case[lit("skewY")]
                >> lit('(')
                >> expr [ _val = construct<skewY_node>(_1) ]
                >> lit(')');

            expr = g.expr.alias();
        }

        typedef qi::locals<expr_node, boost::optional<expr_node>,
                                      boost::optional<expr_node>
                          > rotate_locals;
        typedef qi::rule<Iterator, transform_node(), space_type> node_rule;
        typedef qi::rule<Iterator, transform_list(), space_type> list_rule;

        // rules
        typename expression_grammar<Iterator>::rule_type expr;
        qi::rule<Iterator, transform_list(), space_type> start;
        qi::rule<Iterator, transform_node(), space_type> transform_;
        qi::rule<Iterator, transform_node(), space_type> matrix;
        qi::rule<Iterator, transform_node(), space_type> translate;
        qi::rule<Iterator, transform_node(), space_type> scale;
        qi::rule<Iterator, transform_node(), space_type, rotate_locals> rotate;
        qi::rule<Iterator, transform_node(), space_type> skewX;
        qi::rule<Iterator, transform_node(), space_type> skewY;
    };

} // namespace mapnik

#endif // MAPNIK_TRANSFORM_EXPRESSION_GRAMMAR_HPP
