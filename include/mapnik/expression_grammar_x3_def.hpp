/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_EXPRESSIONS_GRAMMAR_X3_DEF_HPP
#define MAPNIK_EXPRESSIONS_GRAMMAR_X3_DEF_HPP

#include <mapnik/expression_grammar_x3.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/function_call.hpp>
#include <mapnik/json/unicode_string_grammar_x3_def.hpp>
#include <mapnik/unicode.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <mapnik/warning_ignore.hpp>
MAPNIK_DISABLE_WARNING_POP

BOOST_FUSION_ADAPT_STRUCT(mapnik::unary_function_call,
                          (mapnik::unary_function_impl, fun)(mapnik::unary_function_call::argument_type, arg))

BOOST_FUSION_ADAPT_STRUCT(mapnik::binary_function_call,
                          (mapnik::binary_function_impl, fun)(mapnik::binary_function_call::argument_type,
                                                              arg1)(mapnik::binary_function_call::argument_type, arg2))

namespace mapnik {
namespace grammar {

namespace x3 = boost::spirit::x3;
namespace ascii = boost::spirit::x3::ascii;
using ascii::char_;
using x3::_attr;
using x3::_val;
using x3::alnum;
using x3::alpha;
using x3::lexeme;
using x3::lit;
using x3::no_case;
using x3::no_skip;
x3::uint_parser<char, 16, 2, 2> const hex2{};

auto const escaped_unicode = json::grammar::escaped_unicode;

template<typename Context>
inline mapnik::transcoder const& extract_transcoder(Context const& ctx)
{
    return x3::get<transcoder_tag>(ctx);
}

auto const append = [](auto const& ctx) {
    _val(ctx) += _attr(ctx);
};

auto const do_assign = [](auto const& ctx) {
    _val(ctx) = std::move(_attr(ctx));
};

auto const do_negate = [](auto const& ctx) {
    _val(ctx) = std::move(unary_node<mapnik::tags::negate>(_attr(ctx)));
};

auto const do_attribute = [](auto const& ctx) {
    auto const& attr = _attr(ctx);
    if (attr == "mapnik::geometry_type")
    {
        _val(ctx) = std::move(geometry_type_attribute());
    }
    else
    {
        _val(ctx) = std::move(attribute(attr));
    }
};

auto const do_global_attribute = [](auto const& ctx) {
    _val(ctx) = std::move(global_attribute(_attr(ctx)));
};

auto const do_add = [](auto const& ctx) {
    _val(ctx) = std::move(mapnik::binary_node<mapnik::tags::plus>(std::move(_val(ctx)), std::move(_attr(ctx))));
};

auto const do_subt = [](auto const& ctx) {
    _val(ctx) = std::move(mapnik::binary_node<mapnik::tags::minus>(std::move(_val(ctx)), std::move(_attr(ctx))));
};

auto const do_mult = [](auto const& ctx) {
    _val(ctx) = std::move(mapnik::binary_node<mapnik::tags::mult>(std::move(_val(ctx)), std::move(_attr(ctx))));
};

auto const do_div = [](auto const& ctx) {
    _val(ctx) = std::move(mapnik::binary_node<mapnik::tags::div>(std::move(_val(ctx)), std::move(_attr(ctx))));
};

auto const do_mod = [](auto const& ctx) {
    _val(ctx) = std::move(mapnik::binary_node<mapnik::tags::mod>(std::move(_val(ctx)), std::move(_attr(ctx))));
};

auto const do_unicode = [](auto const& ctx) {
    auto const& tr = extract_transcoder(ctx);
    _val(ctx) = std::move(tr.transcode(_attr(ctx).c_str()));
};

auto const do_null = [](auto const& ctx) {
    _val(ctx) = std::move(mapnik::value_null());
};

auto const do_not = [](auto const& ctx) {
    mapnik::unary_node<mapnik::tags::logical_not> node(_attr(ctx));
    _val(ctx) = std::move(node);
};

auto const do_and = [](auto const& ctx) {
    _val(ctx) = std::move(mapnik::binary_node<mapnik::tags::logical_and>(std::move(_val(ctx)), std::move(_attr(ctx))));
};

auto const do_or = [](auto const& ctx) {
    _val(ctx) = std::move(mapnik::binary_node<mapnik::tags::logical_or>(std::move(_val(ctx)), std::move(_attr(ctx))));
};

auto const do_equal = [](auto const& ctx) {
    _val(ctx) = std::move(mapnik::binary_node<mapnik::tags::equal_to>(std::move(_val(ctx)), std::move(_attr(ctx))));
};

auto const do_not_equal = [](auto const& ctx) {
    _val(ctx) = std::move(mapnik::binary_node<mapnik::tags::not_equal_to>(std::move(_val(ctx)), std::move(_attr(ctx))));
};

auto const do_less = [](auto const& ctx) {
    _val(ctx) = std::move(mapnik::binary_node<mapnik::tags::less>(std::move(_val(ctx)), std::move(_attr(ctx))));
};

auto const do_less_equal = [](auto const& ctx) {
    _val(ctx) = std::move(mapnik::binary_node<mapnik::tags::less_equal>(std::move(_val(ctx)), std::move(_attr(ctx))));
};

auto const do_greater = [](auto const& ctx) {
    _val(ctx) = std::move(mapnik::binary_node<mapnik::tags::greater>(std::move(_val(ctx)), std::move(_attr(ctx))));
};

auto const do_greater_equal = [](auto const& ctx) {
    _val(ctx) =
      std::move(mapnik::binary_node<mapnik::tags::greater_equal>(std::move(_val(ctx)), std::move(_attr(ctx))));
};

// regex
auto const do_regex_match = [](auto const& ctx) {
    auto const& tr = extract_transcoder(ctx);
    _val(ctx) = std::move(mapnik::regex_match_node(tr, std::move(_val(ctx)), std::move(_attr(ctx))));
};

auto const do_regex_replace = [](auto const& ctx) {
    auto const& tr = extract_transcoder(ctx);
    auto const& pair = _attr(ctx);
    auto const& pattern = std::get<0>(pair);
    auto const& format = std::get<1>(pair);
    _val(ctx) = mapnik::regex_replace_node(tr, _val(ctx), pattern, format);
};

// mapnik::value_integer
auto const mapnik_int = x3::int_parser<value_integer, 10, 1, -1>();
// mapnik::value_double
auto const mapnik_double = x3::real_parser<value_double, x3::strict_real_policies<value_double>>();
// mapnik::value_bool
struct boolean_ : x3::symbols<mapnik::value_bool>
{
    boolean_()
    {
        add("true", true)  //
          ("false", false) //
          ;
    }
} const boolean;

struct floating_point_constants : x3::symbols<mapnik::value_double>
{
    floating_point_constants()
    {
        add("pi", 3.1415926535897932384626433832795)          //
          ("deg_to_rad", 0.017453292519943295769236907684886) //
          ("rad_to_deg", 57.295779513082320876798154814105)   //
          ;
    }
} const float_const;

// unary functions
struct unary_function_types_ : x3::symbols<unary_function_impl>
{
    unary_function_types_()
    {
        add("sin", sin_impl())      //
          ("cos", cos_impl())       //
          ("tan", tan_impl())       //
          ("atan", atan_impl())     //
          ("exp", exp_impl())       //
          ("log", log_impl())       //
          ("abs", abs_impl())       //
          ("length", length_impl()) //
          ("bool", bool_impl())     //
          ("int", int_impl())       //
          ("float", float_impl())   //
          ("str", str_impl())       //
          ;
    }
} const unary_func_types;

// binary functions

struct binary_function_types_ : x3::symbols<binary_function_impl>
{
    binary_function_types_()
    {
        add("min", binary_function_impl(min_impl)) //
          ("max", binary_function_impl(max_impl))  //
          ("pow", binary_function_impl(pow_impl))  //
          ;
    }
} const binary_func_types;

// geometry types
struct geometry_types_ : x3::symbols<mapnik::value_integer>
{
    geometry_types_()
    {
        add("point", 1)     //
          ("linestring", 2) //
          ("polygon", 3)    //
          ("collection", 4) //
          ;
    }
} const geometry_type;

struct unesc_chars_ : x3::symbols<char>
{
    unesc_chars_()
    {
        add("\\a", '\a') //
          ("\\b", '\b')  //
          ("\\f", '\f')  //
          ("\\n", '\n')  //
          ("\\r", '\r')  //
          ("\\t", '\t')  //
          ("\\v", '\v')  //
          ("\\\\", '\\') //
          ("\\\'", '\'') //
          ("\\\"", '\"') //
          ;
    }
} const unesc_char;

// rules
x3::rule<class logical_expression, mapnik::expr_node> const logical_expression("logical expression");
x3::rule<class not_expression, mapnik::expr_node> const not_expression("not expression");
x3::rule<class conditional_expression, mapnik::expr_node> const conditional_expression("conditional expression");
x3::rule<class equality_expression, mapnik::expr_node> const equality_expression("equality expression");
x3::rule<class relational_expression, mapnik::expr_node> const relational_expression("relational expression");
x3::rule<class additive_expression, mapnik::expr_node> const additive_expression("additive expression");
x3::rule<class multiplicative_expression, mapnik::expr_node> const
  multiplicative_expression("multiplicative expression");
x3::rule<class unary_func_expression, mapnik::unary_function_call> const
  unary_func_expression("unary function expression");
x3::rule<class binary_func_expression, mapnik::binary_function_call> const
  binary_func_expression("binary function expression");
x3::rule<class unary_expression, mapnik::expr_node> const unary_expression("unary expression");
x3::rule<class primary_expression, mapnik::expr_node> const primary_expression("primary expression");
x3::rule<class regex_match_expression, std::string> const regex_match_expression("regex match expression");
x3::rule<class regex_replace_expression, std::pair<std::string, std::string>> const
  regex_replace_expression("regex replace expression");
// clang-format off
    // strings
    auto const single_quoted_string = x3::rule<class single_quoted_string, std::string> {} = lit('\'')
        >> no_skip[*(unesc_char[append]
                     |
                     (lit('\\') >> escaped_unicode[append])
                     |
                     (~char_('\''))[append])] > lit('\'');

    auto const double_quoted_string = x3::rule<class double_quoted_string, std::string> {} = lit('"')
        >> no_skip[*(unesc_char[append]
                     |
                     (lit('\\') >> escaped_unicode[append])
                     |
                     (~char_('"'))[append])] > lit('"');

    auto const quoted_string = x3::rule<class quoted_string, std::string> {} = single_quoted_string | double_quoted_string;

    auto const unquoted_ustring = x3::rule<class ustring, std::string> {} = no_skip[*(unesc_char[append] |
                                                                                   (lit('\\') >> escaped_unicode[append]) |
                                                                                      (~char_(' '))[append])];

    // start
    auto const expression_def = logical_expression [do_assign]
        |
        unquoted_ustring[do_unicode]
        ;

    auto const logical_expression_def = not_expression[do_assign] >
        *(((lit("and") | lit("&&")) > not_expression[do_and])
          |
          ((lit("or") | lit("||")) > not_expression[do_or]));

    auto const not_expression_def = conditional_expression[do_assign]
        |
        ((lit("not") | lit('!')) > conditional_expression[do_not])
        ;

    auto const conditional_expression_def = equality_expression[do_assign]
        |
        additive_expression[do_assign]
        ;

    auto const equality_expression_def = relational_expression[do_assign] >
        *( ( ( lit("=") | lit("eq") | lit("is")) > relational_expression [do_equal])
           | (( lit( "!=") | lit("<>") | lit("neq") ) > relational_expression [do_not_equal])
            );

    auto const relational_expression_def = additive_expression[do_assign] >
        *( ( (lit("<=") | lit("le")) >  additive_expression [do_less_equal])
           |
           ( (lit("<")  | lit("lt")) >> additive_expression[do_less]) // allow backtracking to be able to handle '<' and '<>' correctly
           |
           ( (lit(">=") | lit("ge")) > additive_expression [do_greater_equal])
           |
           ( (lit(">")  | lit("gt")) > additive_expression [do_greater]));


    auto const additive_expression_def = multiplicative_expression[do_assign]
        > *( ('+' > multiplicative_expression[do_add])
             |
             ('-' > multiplicative_expression[do_subt]));

    auto const feature_attr = lexeme['[' > +~char_(']') > ']'];
    auto const global_attr = x3::rule<class global_attr, std::string> {} = lexeme[lit('@') > char_("a-zA-Z_") > *char_("a-zA-Z0-9_")];

    auto const regex_match_expression_def = lit(".match") > '(' > quoted_string > ')';
    auto const regex_replace_expression_def = lit(".replace") > '(' > quoted_string > ',' > quoted_string > ')';
    auto const multiplicative_expression_def = unary_expression [do_assign]
        > *( ('*' > unary_expression [do_mult])
             |
             ('/' > unary_expression [do_div])
             |
             ('%' > unary_expression [do_mod])
             |
             regex_match_expression[do_regex_match]
             |
             regex_replace_expression[do_regex_replace]
            );

    auto const unary_func_expression_def = unary_func_types > '(' > expression > ')';
    auto const binary_func_expression_def = binary_func_types > '(' > expression > ',' > expression > ')';

    auto const unary_expression_def =
        primary_expression[do_assign]
        |
        ('+' > primary_expression[do_assign])
        |
        ('-' > primary_expression[do_negate])
        ;

    auto const primary_expression_def =
        mapnik_double[do_assign]
        |
        mapnik_int[do_assign]
        |
        no_case[boolean][do_assign]
        |
        no_case["null"][do_null]
        |
        no_case[geometry_type][do_assign]
        |
        float_const[do_assign]
        |
        quoted_string[do_unicode]
        |
        feature_attr[do_attribute]
        |
        global_attr[do_global_attribute]
        |
        unary_func_expression[do_assign]
        |
        binary_func_expression[do_assign]
        |
        ('(' > logical_expression[do_assign] > ')')
        ;
// clang-format on
BOOST_SPIRIT_DEFINE(expression,
                    logical_expression,
                    not_expression,
                    conditional_expression,
                    equality_expression,
                    relational_expression,
                    additive_expression,
                    regex_match_expression,
                    regex_replace_expression,
                    multiplicative_expression,
                    unary_func_expression,
                    binary_func_expression,
                    unary_expression,
                    primary_expression);

} // namespace grammar
} // namespace mapnik

#endif // MAPNIK_EXPRESSIONS_GRAMMAR_X3_DEF_HPP
