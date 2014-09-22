/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#include <mapnik/json/feature_generator_grammar.hpp>

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/fusion/include/at.hpp>

namespace mapnik { namespace json {

namespace karma = boost::spirit::karma;

template <typename OutputIterator>
escaped_string<OutputIterator>::escaped_string()
  : escaped_string::base_type(esc_str)
{
    karma::lit_type lit;
    karma::_r1_type _r1;
    karma::hex_type hex;
    karma::right_align_type right_align;
    karma::print_type kprint;
    esc_char.add
        ('"', "\\\"")
        ('\\', "\\\\")
        ('\b', "\\b")
        ('\f', "\\f")
        ('\n', "\\n")
        ('\r', "\\r")
        ('\t', "\\t")
        ;

    esc_str =   lit(_r1)
        << *(esc_char
             | kprint
             | "\\u" << right_align(4,lit('0'))[hex])
        <<  lit(_r1)
        ;
}

template <typename OutputIterator>
feature_generator_grammar<OutputIterator>::feature_generator_grammar()
  : feature_generator_grammar::base_type(feature),
    quote_("\"")
{
    boost::spirit::karma::lit_type lit;
    boost::spirit::karma::uint_type uint_;
    boost::spirit::karma::bool_type bool_;
    boost::spirit::karma::double_type double_;
    boost::spirit::karma::_val_type _val;
    boost::spirit::karma::_1_type _1;
    boost::spirit::karma::string_type kstring;
    boost::spirit::karma::eps_type eps;

    using boost::phoenix::at_c;

    feature = lit("{\"type\":\"Feature\",\"id\":")
        << uint_[_1 = id_(_val)]
        << lit(",\"geometry\":") << geometry
        << lit(",\"properties\":") << properties
        << lit('}')
        ;

    properties = lit('{')
        << -(pair % lit(','))
        << lit('}')
        ;

    pair = lit('"')
        << kstring[_1 = boost::phoenix::at_c<0>(_val)] << lit('"')
        << lit(':')
        << value[_1 = extract_string_(at_c<1>(_val))]
        ;

    value = eps(at_c<1>(_val)) << escaped_string_(quote_.c_str())[_1 = at_c<0>(_val)]
        |
        kstring[_1 = at_c<0>(_val)]
        ;

    // FIXME http://boost-spirit.com/home/articles/karma-examples/creating-your-own-generator-component-for-spirit-karma/
    //value = (value_null_| bool_ | int__ | double_ | ustring)//[_1 = value_base_(_r1)]
    //   ;
    //value_null_ = kstring[_1 = "null"]
    //    ;
    //ustring = escaped_string_(quote_.c_str())[_1 = utf8_(_val)]
    //    ;
}

}}
