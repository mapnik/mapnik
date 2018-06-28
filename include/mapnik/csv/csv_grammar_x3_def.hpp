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

#ifndef MAPNIK_CSV_GRAMMAR_X3_DEF_HPP
#define MAPNIK_CSV_GRAMMAR_X3_DEF_HPP

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <mapnik/csv/csv_grammar_x3.hpp>
#pragma GCC diagnostic pop


namespace mapnik { namespace grammar {

namespace x3 = boost::spirit::x3;

using x3::lit;
using x3::lexeme;
using x3::ascii::char_;

struct unesc_char_ : x3::symbols<char>
{
    unesc_char_()
    {
        add("\\a", '\a')
            ("\\b", '\b')
            ("\\f", '\f')
            ("\\n", '\n')
            ("\\r", '\r')
            ("\\t", '\t')
            ("\\v", '\v')
            ("\\\\",'\\')
            ("\\\'", '\'')
            ("\\\"", '\"')
            ("\"\"", '\"') // double quote
            ;
    }
} unesc_char;

template <typename T>
struct literal : x3::parser<literal<T>>
{
    using attribute_type = x3::unused_type;
    using context_tag = T;
    static bool const has_attribute = false;

    template <typename Iterator, typename Context, typename Attribute>
    bool parse(Iterator& first, Iterator const& last,
               Context const& context, x3::unused_type, Attribute& ) const
    {
        x3::skip_over(first, last, context);
        if (first != last && *first == x3::get<context_tag>(context))
        {
            ++first;
            return true;
        }
        return false;
    }
};

auto static const separator = literal<separator_tag>{};
auto static const quote = literal<quote_tag>{};

// starting rule
csv_line_grammar_type const line("csv-line");
// rules
x3::rule<class csv_column, csv_value> column("csv-column");
x3::rule<class csv_text, csv_value> text("csv-text");
x3::rule<class csc_quoted_text, csv_value> quoted_text("csv-quoted-text");

auto const line_def = -lit('\r') > -lit('\n') > lexeme[column] % separator
    ;

auto const column_def = quoted_text | *(char_ - separator)
    ;

auto const quoted_text_def = quote > text > quote // support unmatched quotes or not (??)
    ;

auto const text_def = *(unesc_char | (char_ - quote))
    ;

BOOST_SPIRIT_DEFINE (
    line,
    column,
    quoted_text,
    text
    );

} // grammar

grammar::csv_line_grammar_type const& csv_line_grammar()
{
    return grammar::line;
}

} // namespace mapnik


#endif // MAPNIK_CSV_GRAMMAR_X3_DEF_HPP
