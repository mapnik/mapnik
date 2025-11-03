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

#include <mapnik/json/properties_generator_grammar.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/phoenix.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {
namespace json {

namespace karma = boost::spirit::karma;

template<typename OutputIterator>
escaped_string<OutputIterator>::escaped_string()
    : escaped_string::base_type(esc_str)
{
    karma::lit_type lit;
    karma::_r1_type _r1;
    karma::char_type char_;
    esc_char.add        //
      ('\a', "\\u0007") //
      ('\b', "\\b")     //
      ('\f', "\\f")     //
      ('\n', "\\n")     //
      ('\r', "\\r")     //
      ('\t', "\\t")     //
      ('\v', "\\u000b") //
      ('"', "\\\"")     //
      ('\\', "\\\\")    //
      ;
    esc_str = lit(_r1)               //
              << *(esc_char | char_) //
              << lit(_r1)            //
      ;
}

template<typename OutputIterator, typename KeyValueStore>
properties_generator_grammar<OutputIterator, KeyValueStore>::properties_generator_grammar()
    : properties_generator_grammar::base_type(properties),
      quote_("\"")
{
    boost::spirit::karma::lit_type lit;
    boost::spirit::karma::_val_type _val;
    boost::spirit::karma::_1_type _1;
    boost::spirit::karma::string_type kstring;
    boost::spirit::karma::eps_type eps;
    using boost::phoenix::at_c;
    // clang-format off
    properties = lit('{')
        << -(pair % lit(','))
        << lit('}')
        ;

    pair = lit('"')
        << kstring[_1 = at_c<0>(_val)] << lit('"')
        << lit(':')
        << value[_1 = extract_string_(at_c<1>(_val))]
        ;

    value = eps(at_c<1>(_val)) << escaped_string_(quote_.c_str())[_1 = at_c<0>(_val)]
        |
        kstring[_1 = at_c<0>(_val)]
        ;
    // clang-format on
}

} // namespace json
} // namespace mapnik
