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

#ifndef MAPNIK_EXPRESSIONS_GRAMMAR_HPP
#define MAPNIK_EXPRESSIONS_GRAMMAR_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/function_call.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_locals.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/fusion/adapted/struct.hpp>
#pragma GCC diagnostic pop

BOOST_FUSION_ADAPT_STRUCT(mapnik::unary_function_call,
                          (mapnik::unary_function_impl, fun)
                          (mapnik::unary_function_call::argument_type, arg))

BOOST_FUSION_ADAPT_STRUCT(mapnik::binary_function_call,
                          (mapnik::binary_function_impl, fun)
                          (mapnik::binary_function_call::argument_type, arg1)
                          (mapnik::binary_function_call::argument_type, arg2))

namespace mapnik
{
namespace qi = boost::spirit::qi;
namespace standard_wide =  boost::spirit::standard_wide;
using standard_wide::space_type;

struct unicode_impl
{
    template <typename T>
    struct result
    {
        using type = mapnik::value_unicode_string;
    };

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
    template <typename T>
    struct result
    {
        using type = expr_node;
    };

    explicit regex_match_impl(mapnik::transcoder const& tr)
        : tr_(tr) {}

    template <typename T0,typename T1>
    expr_node operator() (T0 & node, T1 const& pattern) const;

    mapnik::transcoder const& tr_;
};

struct regex_replace_impl
{

    template <typename T>
    struct result
    {
        using type = expr_node;
    };

    explicit regex_replace_impl(mapnik::transcoder const& tr)
        : tr_(tr) {}

    template <typename T0,typename T1,typename T2>
    expr_node operator() (T0 & node, T1 const& pattern, T2 const& format) const;

    mapnik::transcoder const& tr_;
};

template <typename T>
struct integer_parser
{
    using type = qi::int_parser<T,10,1,-1>;
};

struct unary_function_types : qi::symbols<char, unary_function_impl>
{
    unary_function_types();
};

struct binary_function_types : qi::symbols<char, binary_function_impl>
{
    binary_function_types();
};


#ifdef __GNUC__
template <typename Iterator>
struct MAPNIK_DECL expression_grammar : qi::grammar<Iterator, expr_node(), space_type>
#else
template <typename Iterator>
struct expression_grammar : qi::grammar<Iterator, expr_node(), space_type>
#endif
{
    using rule_type = qi::rule<Iterator, expr_node(), space_type>;

    explicit expression_grammar(std::string const& encoding = "utf-8");

    qi::real_parser<double, qi::strict_real_policies<double> > strict_double;
    typename integer_parser<mapnik::value_integer>::type int__;
    mapnik::transcoder tr_;
    boost::phoenix::function<unicode_impl> unicode_;
    boost::phoenix::function<regex_match_impl> regex_match_;
    boost::phoenix::function<regex_replace_impl> regex_replace_;
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
    qi::rule<Iterator, unary_function_call() , space_type> unary_function_expr;
    qi::rule<Iterator, binary_function_call() , space_type> binary_function_expr;
    qi::rule<Iterator, std::string() > regex_match_expr;
    qi::rule<Iterator, expr_node(expr_node), qi::locals<std::string,std::string>, space_type> regex_replace_expr;
    qi::rule<Iterator, std::string() , space_type> attr;
    qi::rule<Iterator, std::string() , space_type> global_attr;
    qi::rule<Iterator, std::string(), qi::locals<char> > quoted_ustring;
    qi::rule<Iterator, std::string(), space_type> ustring;

    qi::symbols<char const, char const> unesc_char;
    qi::rule<Iterator, char() > quote_char;
    qi::symbols<char, expr_node> constant;
    unary_function_types unary_func_type;
    binary_function_types binary_func_type;

};

} // namespace

#endif  // MAPNIK_EXPRESSIONS_GRAMMAR_HPP
