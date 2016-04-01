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

// NOTE: This is an implementation header file and is only meant to be included
//    from implementation files. It therefore doesn't have an include guard.

// mapnik
#include <mapnik/expression_node.hpp>
#include <mapnik/expression_grammar.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/function_call.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/fusion/adapted/struct.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#pragma GCC diagnostic pop

BOOST_FUSION_ADAPT_STRUCT(mapnik::unary_function_call,
                          (mapnik::unary_function_impl, fun)
                          (mapnik::unary_function_call::argument_type, arg))

BOOST_FUSION_ADAPT_STRUCT(mapnik::binary_function_call,
                          (mapnik::binary_function_impl, fun)
                          (mapnik::binary_function_call::argument_type, arg1)
                          (mapnik::binary_function_call::argument_type, arg2))

// fwd declare
namespace mapnik {
  struct attribute;
  struct geometry_type_attribute;
}

namespace mapnik
{

struct unicode_impl
{
    using result_type = mapnik::value_unicode_string;
    explicit unicode_impl(mapnik::transcoder const& tr)
        : tr_(tr) {}

    mapnik::value_unicode_string operator()(std::string const& str) const
    {
        return tr_.transcode(str.c_str());
    }

    mapnik::transcoder const& tr_;
};

struct regex_match_impl
{
    using result_type = expr_node;
    explicit regex_match_impl(mapnik::transcoder const& tr)
        : tr_(tr) {}

    template <typename T0,typename T1>
    expr_node operator() (T0 & node, T1 const& pattern) const;

    mapnik::transcoder const& tr_;
};

struct regex_replace_impl
{
    using result_type = expr_node;
    explicit regex_replace_impl(mapnik::transcoder const& tr)
        : tr_(tr) {}

    template <typename T0,typename T1,typename T2>
    expr_node operator() (T0 & node, T1 const& pattern, T2 const& format) const;

    mapnik::transcoder const& tr_;
};

unary_function_types::unary_function_types()
{
    add
        ("sin",  sin_impl())
        ("cos",  cos_impl())
        ("tan",  tan_impl())
        ("atan", atan_impl())
        ("exp",  exp_impl())
        ("abs",  abs_impl())
        ("length",length_impl())
        ;
}

binary_function_types::binary_function_types()
{
    add
        ("min", binary_function_impl(min_impl))
        ("max", binary_function_impl(max_impl))
        ("pow", binary_function_impl(pow_impl))
        ;
}

template <typename T0,typename T1>
expr_node regex_match_impl::operator() (T0 & node, T1 const& pattern) const
{
    return regex_match_node(tr_,node,pattern);
}

template <typename T0,typename T1,typename T2>
expr_node regex_replace_impl::operator() (T0 & node, T1 const& pattern, T2 const& format) const
{
    return regex_replace_node(tr_,node,pattern,format);
}

template <typename Iterator>
expression_grammar<Iterator>::expression_grammar(std::string const& encoding)
    : expression_grammar::base_type(expr),
      tr_(encoding)
{
    qi::_1_type _1;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::_r1_type _r1;
    qi::no_skip_type no_skip;
    qi::_val_type _val;
    qi::lit_type lit;
    qi::double_type double_;
    qi::hex_type hex;
    qi::omit_type omit;
    qi::alpha_type alpha;
    qi::alnum_type alnum;
    standard_wide::char_type char_;
    standard_wide::no_case_type no_case;
    using boost::phoenix::construct;
    using boost::phoenix::if_else;

    boost::phoenix::function<unicode_impl> unicode = unicode_impl(tr_);
    boost::phoenix::function<regex_match_impl> regex_match = regex_match_impl(tr_);
    boost::phoenix::function<regex_replace_impl> regex_replace = regex_replace_impl(tr_);

    constant.add
        ("null",        mapnik::value_null())
        ("false",       mapnik::value_bool(false))
        ("true",        mapnik::value_bool(true))
        ("point",       mapnik::value_integer(1))
        ("linestring",  mapnik::value_integer(2))
        ("polygon",     mapnik::value_integer(3))
        ("collection",  mapnik::value_integer(4))
        ("pi",          mapnik::value_double(3.1415926535897932384626433832795))
        ("deg_to_rad",  mapnik::value_double(0.017453292519943295769236907684886))
        ("rad_to_deg",  mapnik::value_double(57.295779513082320876798154814105))
        ;

    expr = logical_expr [_val = _1]
        //| ustring [_val = unicode(_1)]
        ;

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
        >> quoted_ustring [_val = _1]
        >> lit(')')
        ;

    regex_replace_expr =
        lit(".replace")
        >> lit('(')
        >> quoted_ustring           [_a = _1]
        >> lit(',')
        >> quoted_ustring           [_b = _1]
        >> lit(')')          [_val = regex_replace(_r1,_a,_b)]
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
                  | '%' >> unary_expr [_val %= construct<mapnik::expr_node>(_1)] //needed by clang++ with -std=c++11
                  |  regex_match_expr[_val = regex_match(_val, _1)]
                  |  regex_replace_expr(_val) [_val = _1]
            )
        ;

    unary_function_expr = unary_func_type >> '(' > logical_expr > ')'
        ;

    binary_function_expr = binary_func_type >> '(' > logical_expr > ','
                                                   > logical_expr > ')'
        ;

    unary_expr = primary_expr [_val = _1]
        | '+' >> primary_expr [_val = _1]
        | '-' >> primary_expr [_val = -_1]
        ;

    primary_expr = strict_double [_val = _1]
        | int__[_val = _1]
        | no_case[constant] [_val = _1]
        | quoted_ustring [_val = unicode(_1)]
        | attr [if_else(_1 == "mapnik::geometry_type",
                        _val = construct<mapnik::geometry_type_attribute>(),
                        _val = construct<mapnik::attribute>(_1))]
        | global_attr [_val = construct<mapnik::global_attribute>( _1 )]
        | unary_function_expr [_val = _1]
        | binary_function_expr [_val = _1]
        | '(' > logical_expr [_val = _1 ] > ')'
        //  TODO: this is a backward compatibility hack to allow unquoted strings
        | unquoted_ustring [_val = unicode(_1)]
        // ^ https://github.com/mapnik/mapnik/pull/3389
        ;

    unesc_char.add("\\a", '\a')("\\b", '\b')("\\f", '\f')("\\n", '\n')
        ("\\r", '\r')("\\t", '\t')("\\v", '\v')("\\\\", '\\')
        ("\\\'", '\'')("\\\"", '\"')
        ;

    ustring %= no_skip[alpha >> *alnum];
    quote_char %= char_('\'') | char_('"');
    quoted_ustring %= omit[quote_char[_a = _1]]
        >> *(unesc_char | "\\x" >> hex | (char_ - lit(_a)))
        >> lit(_a);
    unquoted_ustring %= no_skip[alpha >> *alnum] - lit("not");
    attr %= '[' >> no_skip[+~char_(']')] >> ']';
    global_attr %= '@' >> no_skip[alpha >> * (alnum | char_('-'))];

}

}
