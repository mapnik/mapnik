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
#include <mapnik/feature.hpp>
#include <mapnik/json/feature_grammar.hpp>

// boost
#include <boost/spirit/include/support_multi_pass.hpp>

namespace mapnik { namespace json {

template <typename Iterator, typename FeatureType>
feature_grammar<Iterator,FeatureType>::feature_grammar(generic_json<Iterator> & json,  mapnik::transcoder const& tr)
    : feature_grammar::base_type(feature,"feature"),
      json_(json),
      put_property_(put_property(tr))
{
    qi::lit_type lit;
    qi::long_long_type long_long;
    qi::double_type double_;
    qi::no_skip_type no_skip;
    standard_wide::char_type char_;
    qi::_val_type _val;
    qi::_1_type _1;
    qi::_2_type _2;
    qi::_3_type _3;
    qi::_4_type _4;
    qi::_a_type _a;
    qi::_r1_type _r1;
    qi::eps_type eps;

    using qi::fail;
    using qi::on_error;
    using phoenix::new_;
    using phoenix::push_back;
    using phoenix::construct;

    // generic json types
    json_.value =  json_.object | json_.array | json_.string_
        | json_.number
        ;

    json_.pairs = json_.key_value % lit(',')
        ;

    json_.key_value = (json_.string_ >> lit(':') >> json_.value)
        ;

    json_.object = lit('{')
        >> *json_.pairs
        >> lit('}')
        ;
    json_.array = lit('[')
        >> json_.value >> *(lit(',') >> json_.value)
        >> lit(']')
        ;
    json_.number %= json_.strict_double
        | json_.int__
        | lit("true") [_val = true]
        | lit ("false") [_val = false]
        | lit("null")[_val = construct<value_null>()]
        ;

    json_.unesc_char.add
        ("\\\"", '\"') // quotation mark
        ("\\\\", '\\') // reverse solidus
        ("\\/", '/')   // solidus
        ("\\b", '\b')  // backspace
        ("\\f", '\f')  // formfeed
        ("\\n", '\n')  // newline
        ("\\r", '\r')  // carrige return
        ("\\t", '\t')  // tab
        ;

    json_.string_ %= lit('"') >> no_skip[*(json_.unesc_char | "\\u" >> json_.hex4 | (char_ - lit('"')))] >> lit('"')
        ;

    // geojson types
    feature_type = lit("\"type\"")
        >> lit(':')
        >> lit("\"Feature\"")
        ;

    feature = lit('{')
        >> (feature_type | (lit("\"geometry\"") >> lit(':')
                            >> geometry_grammar_(extract_geometry_(_r1))) | properties(_r1) | json_.key_value) % lit(',')
        >> lit('}')
        ;

    properties = lit("\"properties\"")
        >> lit(':') >> (lit('{') >>  attributes(_r1) >> lit('}')) | lit("null")
        ;

    attributes = (json_.string_ [_a = _1] >> lit(':') >> attribute_value [put_property_(_r1,_a,_1)]) % lit(',')
        ;

    attribute_value %= json_.number | json_.string_  ;

    feature.name("Feature");
    properties.name("Properties");
    attributes.name("Attributes");

    on_error<fail>
        (
            feature
            , std::clog
            << phoenix::val("Error! Expecting ")
            << _4 // what failed?
            << phoenix::val(" here: \"")
            << where_message_(_3, _2, 16) // where? 16 is max chars to output
            << phoenix::val("\"")
            << std::endl
            );

}

template struct mapnik::json::feature_grammar<std::string::const_iterator,mapnik::feature_impl>;
template struct mapnik::json::feature_grammar<boost::spirit::multi_pass<std::istreambuf_iterator<char> >,mapnik::feature_impl>;

}}
