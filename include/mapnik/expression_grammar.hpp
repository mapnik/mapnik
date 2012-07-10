/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_EXPRESSIONS_GRAMMAR_HPP
#define MAPNIK_EXPRESSIONS_GRAMMAR_HPP

// mapnik
#include <mapnik/value.hpp>
#include <mapnik/expression_node.hpp>

// boost
#include <boost/version.hpp>
#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/concept_check.hpp>

// spirit2
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_action.hpp>

// fusion
#include <boost/fusion/include/adapt_struct.hpp>

// phoenix
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>

namespace mapnik
{

using namespace boost;
namespace qi = boost::spirit::qi;
namespace standard_wide =  boost::spirit::standard_wide;
using standard_wide::space_type;

struct unicode_impl
{
    template <typename T>
    struct result
    {
        typedef UnicodeString type;
    };

    explicit unicode_impl(mapnik::transcoder const& tr)
        : tr_(tr) {}

    UnicodeString operator()(std::string const& str) const
    {
        return tr_.transcode(str.c_str());
    }

    mapnik::transcoder const& tr_;
};

struct regex_match_impl
{
    template <typename T0, typename T1>
    struct result
    {
        typedef expr_node type;
    };

    explicit regex_match_impl(mapnik::transcoder const& tr)
        : tr_(tr) {}

    template <typename T0,typename T1>
    expr_node operator() (T0 & node, T1 const& pattern) const
    {
#if defined(BOOST_REGEX_HAS_ICU)
        return regex_match_node(node,tr_.transcode(pattern.c_str()));
#else
        return regex_match_node(node,pattern);
#endif
    }

    mapnik::transcoder const& tr_;
};

struct regex_replace_impl
{
    template <typename T0, typename T1, typename T2>
    struct result
    {
        typedef expr_node type;
    };

    explicit regex_replace_impl(mapnik::transcoder const& tr)
        : tr_(tr) {}

    template <typename T0,typename T1,typename T2>
    expr_node operator() (T0 & node, T1 const& pattern, T2 const& format) const
    {
#if defined(BOOST_REGEX_HAS_ICU)
        return regex_replace_node(node,tr_.transcode(pattern.c_str()),tr_.transcode(format.c_str()));
#else
        return regex_replace_node(node,pattern,format);
#endif
    }

    mapnik::transcoder const& tr_;
};

template <typename Iterator>
struct expression_grammar : qi::grammar<Iterator, expr_node(), space_type>
{
    typedef qi::rule<Iterator, expr_node(), space_type> rule_type;

    explicit expression_grammar(mapnik::transcoder const& tr)
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
            | lit("true") [_val = true]
            | lit("false") [_val = false]
            | lit("null") [_val = value_null() ]
            | ustring [_val = unicode_(_1) ]
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

    qi::real_parser<double, qi::strict_real_policies<double> > strict_double;
    boost::phoenix::function<unicode_impl> unicode_;
    boost::phoenix::function<regex_match_impl> regex_match_;
    boost::phoenix::function<regex_replace_impl> regex_replace_;
    //
    rule_type expr;
    rule_type equality_expr;
    rule_type cond_expr;
    rule_type relational_expr;
    rule_type logical_expr;
    rule_type additive_expr;
    rule_type multiplicative_expr;
    rule_type unary_expr;
    rule_type not_expr;
    rule_type primary_expr;
    qi::rule<Iterator, std::string() > regex_match_expr;
    qi::rule<Iterator, expr_node(expr_node), qi::locals<std::string,std::string>, space_type> regex_replace_expr;
    qi::rule<Iterator, std::string() , space_type> attr;
    qi::rule<Iterator, std::string(), qi::locals<char> > ustring;
    qi::symbols<char const, char const> unesc_char;
    qi::rule<Iterator, char() > quote_char;
};

} // namespace

#endif  // MAPNIK_EXPRESSIONS_GRAMMAR_HPP
