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

#include <boost/version.hpp>
#if BOOST_VERSION >= 104700

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/json/feature_grammar.hpp>

// boost
#include <boost/spirit/include/support_multi_pass.hpp>

namespace mapnik { namespace json {

template <typename Iterator, typename FeatureType>
feature_grammar<Iterator,FeatureType>::feature_grammar(mapnik::transcoder const& tr)
    : feature_grammar::base_type(feature,"feature"),
      put_property_(put_property(tr))
{
    using qi::lit;
    using qi::int_;
    using qi::double_;
#if BOOST_VERSION > 104200
    using qi::no_skip;
#else
    using qi::lexeme;
#endif
    using standard_wide::char_;
    using qi::_val;
    using qi::_1;
    using qi::_2;
    using qi::_3;
    using qi::_4;
    using qi::_a;
    using qi::_b;
    using qi::_r1;
    using qi::_r2;
    using qi::fail;
    using qi::on_error;
    using qi::_pass;
    using qi::eps;
    using qi::raw;

    using phoenix::new_;
    using phoenix::push_back;
    using phoenix::construct;

    // generic json types
    value =  object | array | string_
        | number
        ;

    pairs = key_value % lit(',')
        ;

    key_value = (string_ >> lit(':') >> value)
        ;

    object = lit('{')
        >> *pairs
        >> lit('}')
        ;
    array = lit('[')
        >> value >> *(lit(',') >> value)
        >> lit(']')
        ;
// https://github.com/mapnik/mapnik/issues/1342
#if BOOST_VERSION >= 104700
    number %= strict_double
#else
    number = strict_double
#endif
        | int_
        | lit("true") [_val = true]
        | lit ("false") [_val = false]
        | lit("null")[_val = construct<value_null>()]
        ;

    unesc_char.add
        ("\\\"", '\"') // quotation mark
        ("\\\\", '\\') // reverse solidus
        ("\\/", '/')   // solidus
        ("\\b", '\b')  // backspace
        ("\\f", '\f')  // formfeed
        ("\\n", '\n')  // newline
        ("\\r", '\r')  // carrige return
        ("\\t", '\t')  // tab
        ;
#if BOOST_VERSION > 104200
    string_ %= lit('"') >> no_skip[*(unesc_char | "\\u" >> hex4 | (char_ - lit('"')))] >> lit('"')
        ;
#else
    string_ %= lit('"') >> lexeme[*(unesc_char | "\\u" >> hex4 | (char_ - lit('"')))] >> lit('"')
        ;
#endif
    // geojson types

    feature_type = lit("\"type\"")
        >> lit(':')
        >> lit("\"Feature\"")
        ;

    feature = lit('{')
        >> (feature_type | (lit("\"geometry\"") > lit(':') > geometry_grammar_(extract_geometry_(_r1))) | properties(_r1) | key_value) % lit(',')
        >> lit('}')
        ;

    properties = lit("\"properties\"")
        >> lit(':') >> (lit('{') >>  attributes(_r1) >> lit('}')) | lit("null")
        ;

    attributes = (string_ [_a = _1] >> lit(':') >> attribute_value [put_property_(_r1,_a,_1)]) % lit(',')
        ;

    attribute_value %= number | string_  ;


    on_error<fail>
        (
            feature
            , std::clog
            << phoenix::val("Error! Expecting ")
            << _4                               // what failed?
            << phoenix::val(" here: \"")
            << construct<std::string>(_3, _2)   // iterators to error-pos, end
            << phoenix::val("\"")
            << std::endl
            );

}

template struct mapnik::json::feature_grammar<std::string::const_iterator,mapnik::Feature>;
template struct mapnik::json::feature_grammar<boost::spirit::multi_pass<std::istreambuf_iterator<char> >,mapnik::Feature>;

}}

#endif
