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

// mapnik
#include <mapnik/transform_expression_grammar.hpp>

// boost
#include <boost/version.hpp>

// spirit
#include <boost/spirit/home/phoenix/object/construct.hpp>

namespace mapnik {

namespace qi = boost::spirit::qi;

template <typename Iterator>
transform_expression_grammar<Iterator>::transform_expression_grammar(expression_grammar<Iterator> const& g)
        : transform_expression_grammar::base_type(start)
{
    using boost::phoenix::construct;
    using qi::_a; using qi::_1; using qi::_4;
    using qi::_b; using qi::_2; using qi::_5;
    using qi::_c; using qi::_3; using qi::_6;
    using qi::_val;
    using qi::char_;
    using qi::double_;
    using qi::lit;
    using qi::no_case;

    // [http://www.w3.org/TR/SVG/coords.html#TransformAttribute]

    // The value of the ‘transform’ attribute is a <transform-list>, which
    // is defined as a list of transform definitions, which are applied in
    // the order provided.  The individual transform definitions are
    // separated by whitespace and/or a comma.

#if BOOST_VERSION > 104200
    using qi::no_skip;
    start = transform_ % no_skip[char_(", ")] ;
#else
    using qi::lexeme;
    start = transform_ % lexeme[char_(", ")] ;
#endif

    transform_ = matrix | translate | scale | rotate | skewX | skewY ;

    // matrix(<a> <b> <c> <d> <e> <f>)
    matrix = no_case[lit("matrix")]
        >> (lit('(')
            >> ( atom >> sep_atom >> sep_atom >> sep_atom >> sep_atom
                      >> sep_atom >> lit(')')
               | expr >> sep_expr >> sep_expr >> sep_expr >> sep_expr
                      >> sep_expr >> lit(')')
               ))
        [ _val = construct<matrix_node>(_1,_2,_3,_4,_5,_6) ];

    // translate(<tx> [<ty>])
    translate = no_case[lit("translate")]
        >> lit('(')
        >> ( ( atom >> -sep_atom >> lit(')') )
             [ _val = construct<translate_node>(_1,_2) ]
           | ( expr >> -sep_expr >> lit(')') )
             [ _val = construct<translate_node>(_1,_2) ]
           );

    // scale(<sx> [<sy>])
    scale = no_case[lit("scale")]
        >> lit('(')
        >> ( ( atom >> -sep_atom >> lit(')') )
             [ _val = construct<scale_node>(_1,_2) ]
           | ( expr >> -sep_expr >> lit(')') )
             [ _val = construct<scale_node>(_1,_2) ]
           );

    // rotate(<rotate-angle> [<cx> <cy>])
    rotate = no_case[lit("rotate")]
        >> lit('(')
        >> ( ( atom >> -( sep_atom >> sep_atom ) >> lit(')') )
             [ _val = construct<rotate_node>(_1,_2) ]
           | ( expr >> -( sep_expr >> sep_expr ) >> lit(')') )
             [ _val = construct<rotate_node>(_1,_2) ]
           );

    // skewX(<skew-angle>)
    skewX = no_case[lit("skewX")]
        >> lit('(')
        >> expr [ _val = construct<skewX_node>(_1) ]
        >> lit(')');

    // skewY(<skew-angle>)
    skewY = no_case[lit("skewY")]
        >> lit('(')
        >> expr [ _val = construct<skewY_node>(_1) ]
        >> lit(')');

    // number or attribute
    atom = double_ [ _val = _1 ]
        | attr [ _val = construct<mapnik::attribute>(_1) ];

    // Individual arguments in lists consiting solely of numbers and/or
    // attributes are separated by whitespace and/or a comma.
    sep_atom = -lit(',') >> atom [ _val = _1 ];

    // Individual arguments in lists containing one or more compound
    // expressions are separated by a comma.
    sep_expr = lit(',') >> expr [ _val = _1 ];

    attr = g.attr.alias();
    expr = g.expr.alias();
}

template struct mapnik::transform_expression_grammar<std::string::const_iterator>;

}
