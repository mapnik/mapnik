/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_GENERIC_JSON_HPP
#define MAPNIK_GENERIC_JSON_HPP

#include <mapnik/value_types.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/json/value_converters.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/regex/pending/unicode_iterator.hpp>
#pragma GCC diagnostic pop

#include <vector>

namespace mapnik { namespace json {

namespace qi = boost::spirit::qi;
namespace standard = boost::spirit::standard;
namespace phoenix = boost::phoenix;
using space_type = standard::space_type;

struct json_value;

using json_array = std::vector<json_value>;
using json_object_element = std::pair<std::string, json_value>;
using json_object = std::vector<json_object_element>;
using json_value_base = mapnik::util::variant<value_null,
                                              value_bool,
                                              value_integer,
                                              value_double,
                                              std::string,
                                              mapnik::util::recursive_wrapper<json_array>,
                                              mapnik::util::recursive_wrapper<json_object> >;
struct json_value : json_value_base
{
#if __cpp_inheriting_constructors >= 200802

    using json_value_base::json_value_base;

#else

    json_value() = default;

    template <typename T>
    json_value(T && val)
        : json_value_base(std::forward<T>(val)) {}

#endif
};

using uchar = std::uint32_t; // a unicode code point

// unicode string grammar via boost/libs/spirit/example/qi/json/json/parser/grammar.hpp

template <typename Iterator>
struct unicode_string : qi::grammar<Iterator, std::string()>
{
    unicode_string();
    qi::rule<Iterator, void(std::string&)> escape;
    qi::rule<Iterator, void(std::string&)> char_esc;
    qi::rule<Iterator, std::string()> double_quoted;
};


struct push_utf8
{
    using result_type = void;

    void operator()(std::string& utf8, uchar code_point) const
    {
        typedef std::back_insert_iterator<std::string> insert_iter;
        insert_iter out_iter(utf8);
        boost::utf8_output_iterator<insert_iter> utf8_iter(out_iter);
        *utf8_iter++ = code_point;
    }
};

struct push_esc
{
    using result_type = void;

    void operator()(std::string& utf8, uchar c) const
    {
        switch (c)
        {
        case ' ': utf8 += ' ';          break;
        case '\t': utf8 += '\t';        break;
        case '0': utf8 += char(0);      break;
        case 'a': utf8 += 0x7;          break;
        case 'b': utf8 += 0x8;          break;
        case 't': utf8 += 0x9;          break;
        case 'n': utf8 += 0xA;          break;
        case 'v': utf8 += 0xB;          break;
        case 'f': utf8 += 0xC;          break;
        case 'r': utf8 += 0xD;          break;
        case 'e': utf8 += 0x1B;         break;
        case '"': utf8 += '"';          break;
        case '/': utf8 += '/';          break;
        case '\\': utf8 += '\\';        break;
        case '_': push_utf8()(utf8, 0xA0);  break;
        case 'N': push_utf8()(utf8, 0x85);  break;
        case 'L': push_utf8()(utf8, 0x2028);  break;
        case 'P': push_utf8()(utf8, 0x2029);  break;
        }
    }
};

template< typename Iterator>
unicode_string<Iterator>::unicode_string()
    : unicode_string::base_type(double_quoted)
{
    qi::char_type char_;
    qi::_val_type _val;
    qi::_r1_type _r1;
    qi::_1_type _1;
    qi::lit_type lit;
    qi::eol_type eol;
    qi::repeat_type repeat;
    qi::hex_type hex;

    using boost::spirit::qi::uint_parser;
    using boost::phoenix::function;
    using boost::phoenix::ref;

    uint_parser<uchar, 16, 4, 4> hex4;
    uint_parser<uchar, 16, 8, 8> hex8;
    function<push_utf8> push_utf8;
    function<push_esc> push_esc;

    escape =
        ('x' > hex)                     [push_utf8(_r1, _1)]
        |
        ('u' > hex4)                    [push_utf8(_r1, _1)]
        |
        ('U' > hex8)                    [push_utf8(_r1, _1)]
        |
        char_("0abtnvfre\"/\\N_LP \t")  [push_esc(_r1, _1)]
        |
        eol                             // continue to next line
        ;

    char_esc =
        '\\' > escape(_r1)
        ;

    double_quoted =
        '"'
        > *(char_esc(_val) | (~char_('"'))    [_val += _1])
        > '"'
        ;
}

template <typename Iterator>
struct generic_json : qi::grammar<Iterator, json_value(), space_type>
{
    generic_json();
    qi::rule<Iterator, json_value(), space_type> value;
    qi::int_parser<mapnik::value_integer, 10, 1, -1> int__;
    unicode_string<Iterator> string_;
    qi::rule<Iterator, json_object_element(), space_type> key_value;
    qi::rule<Iterator, json_value(), space_type> number;
    qi::rule<Iterator, json_object(), space_type> object;
    qi::rule<Iterator, json_array(), space_type> array;
    qi::real_parser<double, qi::strict_real_policies<double>> strict_double;
    // conversions
    boost::phoenix::function<mapnik::detail::value_converter<mapnik::value_integer>> integer_converter;
    boost::phoenix::function<mapnik::detail::value_converter<mapnik::value_double>> double_converter;
};

}}

#endif // MAPNIK_GENERIC_JSON_HPP
