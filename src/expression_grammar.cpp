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

#include <mapnik/expression_grammar.hpp>

namespace mapnik
{

template <typename T0,typename T1>
expr_node regex_match_impl::operator() (T0 & node, T1 const& pattern) const
{
#if defined(BOOST_REGEX_HAS_ICU)
    return regex_match_node(node,tr_.transcode(pattern.c_str()));
#else
    return regex_match_node(node,pattern);
#endif
}

template <typename T0,typename T1,typename T2>
expr_node regex_replace_impl::operator() (T0 & node, T1 const& pattern, T2 const& format) const
{
#if defined(BOOST_REGEX_HAS_ICU)
    return regex_replace_node(node,tr_.transcode(pattern.c_str()),tr_.transcode(format.c_str()));
#else
    return regex_replace_node(node,pattern,format);
#endif
}

template <typename Iterator>
expression_grammar<Iterator>::expression_grammar(mapnik::transcoder const& tr)
        : expression_grammar::base_type(expr),
          unicode_(unicode_impl(tr)),
          regex_match_(regex_match_impl(tr)),
          regex_replace_(regex_replace_impl(tr))
{
    using boost::phoenix::construct;
    using qi::_1;
    using qi::_a;
    using qi::_b;
    using qi::_r1;
#if BOOST_VERSION > 104200
    using qi::no_skip;
#endif
    using qi::lexeme;
    using qi::_val;
    using qi::lit;
    using qi::int_;
    using qi::double_;
    using qi::hex;
    using qi::omit;
    using standard_wide::char_;
    using standard_wide::no_case;
    expr = logical_expr.alias();

    logical_expr = not_expr [_val = _1]
        >>
        *(  (  ( lit("and") | lit("&&")) >> not_expr [_val && _1] )
            | (( lit("or") | lit("||")) >> not_expr [_val || _1])
            )
        ;

    not_expr =
        cond_expr [_val = _1 ]
        | ((lit("not") | lit('!')) >> cond_expr [ _val = !_1 ])
        ;

    cond_expr = equality_expr [_val = _1] | additive_expr [_val = _1]
        ;

    equality_expr =
        relational_expr [_val = _1]
        >> *(  ( (lit("=") | lit("eq") | lit("is")) >> relational_expr [_val == _1])
               | (( lit("!=") | lit("<>") | lit("neq") ) >> relational_expr [_val != _1])
            )
        ;

    regex_match_expr = lit(".match")
        >> lit('(')
        >> ustring [_val = _1]
        >> lit(')')
        ;

    regex_replace_expr =
        lit(".replace")
        >> lit('(')
        >> ustring           [_a = _1]
        >> lit(',')
        >> ustring           [_b = _1]
        >> lit(')')          [_val = regex_replace_(_r1,_a,_b)]
        ;

    relational_expr = additive_expr[_val = _1]
        >>
        *(  (   (lit("<=") | lit("le") ) >> additive_expr [ _val <= _1 ])
            | ( (lit('<')  | lit("lt") ) >> additive_expr [ _val <  _1 ])
            | ( (lit(">=") | lit("ge") ) >> additive_expr [ _val >= _1 ])
            | ( (lit('>')  | lit("gt") ) >> additive_expr [ _val >  _1 ])
            )
        ;

    additive_expr = multiplicative_expr [_val = _1]
        >> * (   '+' >> multiplicative_expr[_val += _1]
                 | '-' >> multiplicative_expr[_val -= _1]
            )
        ;

    multiplicative_expr = unary_expr [_val = _1]
        >> *(     '*' >> unary_expr [_val *= _1]
                  | '/' >> unary_expr [_val /= _1]
                  | '%' >> unary_expr [_val %= _1]
                  |  regex_match_expr[_val = regex_match_(_val, _1)]
                  |  regex_replace_expr(_val) [_val = _1]
            )
        ;

    unary_expr = primary_expr [_val = _1]
        | '+' >> primary_expr [_val = _1]
        | '-' >> primary_expr [_val = -_1]
        ;

    primary_expr = strict_double [_val = _1]
        | int_ [_val = _1]
        | no_case[lit("true")] [_val = true]
        | no_case[lit("false")] [_val = false]
        | no_case[lit("null")] [_val = value_null() ]
        | no_case[geom_type][_val = _1 ]
        | ustring [_val = unicode_(_1) ]
        | lit("[mapnik::geometry_type]")[_val = construct<mapnik::geometry_type_attribute>()]
        | attr [_val = construct<mapnik::attribute>( _1 ) ]
        | '(' >> expr [_val = _1 ] >> ')'
        ;

    unesc_char.add("\\a", '\a')("\\b", '\b')("\\f", '\f')("\\n", '\n')
        ("\\r", '\r')("\\t", '\t')("\\v", '\v')("\\\\", '\\')
        ("\\\'", '\'')("\\\"", '\"')
        ;

#if BOOST_VERSION > 104500
    quote_char %= char_('\'') | char_('"');
    ustring %= omit[quote_char[_a = _1]]
        >> *(unesc_char | "\\x" >> hex | (char_ - lit(_a)))
        >> lit(_a);
    attr %= '[' >> no_skip[+~char_(']')] >> ']';
#else
    ustring %= lit('\'')
        >> *(unesc_char | "\\x" >> hex | (char_ - lit('\'')))
        >> lit('\'');
    attr %= '[' >> lexeme[+(char_ - ']')] >> ']';
#endif

}

template struct mapnik::expression_grammar<std::string::const_iterator>;

}