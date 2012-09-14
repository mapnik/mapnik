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
    expr_node operator() (T0 & node, T1 const& pattern) const;

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
    expr_node operator() (T0 & node, T1 const& pattern, T2 const& format) const;

    mapnik::transcoder const& tr_;
};

struct geometry_types : qi::symbols<char,int>
{
    geometry_types()
    {
        add
            ("point",1)
            ("linestring", 2)
            ("polygon",3)
            ("collection",4)
            ;
    }
};

template <typename Iterator>
struct expression_grammar : qi::grammar<Iterator, expr_node(), space_type>
{
    typedef qi::rule<Iterator, expr_node(), space_type> rule_type;

    explicit expression_grammar(mapnik::transcoder const& tr);

    qi::real_parser<double, qi::strict_real_policies<double> > strict_double;
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
    qi::rule<Iterator, std::string() > regex_match_expr;
    qi::rule<Iterator, expr_node(expr_node), qi::locals<std::string,std::string>, space_type> regex_replace_expr;
    qi::rule<Iterator, std::string() , space_type> attr;
    qi::rule<Iterator, std::string(), qi::locals<char> > ustring;
    qi::symbols<char const, char const> unesc_char;
    qi::rule<Iterator, char() > quote_char;
    geometry_types geom_type;
};

} // namespace

#endif  // MAPNIK_EXPRESSIONS_GRAMMAR_HPP
