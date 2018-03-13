/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_JSON_UNICODE_STRING_GRAMMAR_X3_DEF_HPP
#define MAPNIK_JSON_UNICODE_STRING_GRAMMAR_X3_DEF_HPP

#include <mapnik/json/unicode_string_grammar_x3.hpp>
#include <mapnik/init_priority.hpp>
// boost
#include <boost/regex/pending/unicode_iterator.hpp>
//
namespace mapnik { namespace json { namespace grammar {

namespace x3 = boost::spirit::x3;

using uchar = std::uint32_t; // a unicode code point

auto append = [](auto const& ctx)
{
    _val(ctx) += _attr(ctx);
};

namespace detail {

void push_utf8_impl(std::string & str, uchar code_point)
{
    using insert_iterator = std::back_insert_iterator<std::string>;
    insert_iterator iter(str);
    boost::utf8_output_iterator<insert_iterator> utf8_iter(iter);
    *utf8_iter++ = code_point;
}
}

auto push_char = [](auto const& ctx) { _val(ctx).push_back(_attr(ctx));};

auto push_utf8 = [](auto const& ctx) { detail::push_utf8_impl(_val(ctx), _attr(ctx));};

auto push_utf16 = [](auto const& ctx)
{
    using iterator_type = std::vector<std::uint16_t>::const_iterator;
    auto const& utf16 = _attr(ctx);
    try
    {
        boost::u16_to_u32_iterator<iterator_type> itr(utf16.begin());
        boost::u16_to_u32_iterator<iterator_type> end(utf16.end());
        for (; itr != end; ++itr)
        {
            detail::push_utf8_impl(_val(ctx), *itr);
        }
    }
    catch( ... )
    {
        // caught
    }
};

auto push_esc = [] (auto const& ctx)
{
    std::string & utf8 = _val(ctx);
    char c = _attr(ctx);
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
    case '_': detail::push_utf8_impl(utf8, 0xA0);  break;
    case 'N': detail::push_utf8_impl(utf8, 0x85);  break;
    case 'L': detail::push_utf8_impl(utf8, 0x2028);  break;
    case 'P': detail::push_utf8_impl(utf8, 0x2029);  break;
    }
};

using x3::lit;
using x3::char_;
using x3::eol;
using x3::no_skip;

x3::uint_parser<char,  16, 2, 2> const hex2 {};
x3::uint_parser<std::uint16_t, 16, 4, 4> const hex4 {};
x3::uint_parser<uchar, 16, 8, 8> const hex8 {};

// start rule
unicode_string_grammar_type const unicode_string MAPNIK_INIT_PRIORITY(101) ("Unicode String");
escaped_unicode_type const escaped_unicode ("Escaped Unicode code point(s)");
// rules
x3::rule<class double_quoted_tag, std::string> const double_quoted("Double-quoted string");
x3::rule<class escaped_tag, std::string> const escaped("Escaped Character");
x3::rule<class utf16_string_tag, std::vector<std::uint16_t>> const utf16_string("UTF16 encoded string");

auto unicode_string_def = double_quoted
    ;
auto utf16_string_def = lit('u') > hex4 > *(lit("\\u") > hex4)
    ;
auto escaped_unicode_def =
    (lit('x') > hex2[push_char])
    |
    utf16_string[push_utf16]
    |
    (lit('U') > hex8[push_utf8])
    ;
auto const escaped_def = lit('\\') >
    (escaped_unicode[append]
     |
     char_("0abtnvfre\"/\\N_LP \t")[push_esc]
     |
     eol) // continue to next line
    ;
auto const double_quoted_def = lit('"') > no_skip[*(escaped[append] | (~char_('"'))[append])] > lit('"');

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>

BOOST_SPIRIT_DEFINE(
    unicode_string,
    double_quoted,
    escaped,
    escaped_unicode,
    utf16_string
    );

#pragma GCC diagnostic pop

}}}

#endif // MAPNIK_JSON_UNICODE_STRING_GRAMMAR_X3_DEF_HPP
