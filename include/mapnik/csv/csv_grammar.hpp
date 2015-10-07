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

#ifndef MAPNIK_CVS_GRAMMAR_HPP
#define MAPNIK_CVS_GRAMMAR_HPP

//#define BOOST_SPIRIT_DEBUG

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace mapnik {

namespace qi = boost::spirit::qi;
using csv_value  = std::string;
using csv_line = std::vector<csv_value>;
using csv_data = std::vector<csv_line>;

template <typename Iterator>
struct csv_white_space_skipper : qi::grammar<Iterator>
{
    csv_white_space_skipper()
        : csv_white_space_skipper::base_type(skip)
    {
        using namespace qi;
        qi::lit_type lit;
        skip = +lit(' ')
            ;
    }
    qi::rule<Iterator> skip;
};

template <typename Iterator, typename Skipper = csv_white_space_skipper<Iterator> >
struct csv_line_grammar : qi::grammar<Iterator, csv_line(char, char), Skipper>
{
    csv_line_grammar()
        : csv_line_grammar::base_type(line)
    {
        using namespace qi;
        qi::_a_type _a;
        qi::_r1_type _r1;
        qi::_r2_type _r2;
        qi::lit_type lit;
        qi::_1_type _1;
        qi::char_type char_;
        qi::omit_type omit;
        unesc_char.add
            ("\\a", '\a')
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
        line = -omit[char_("\n\r")] >> column(_r1, _r2) % lit(_r1)
            ;
        column = quoted(_r2) | *(char_ - (lit(_r1)))
            ;
        quoted = omit[char_(_r1)[_a = _1]] > text(_a) > -lit(_a) // support unmatched quotes or not (??)
            ;
        text = *(unesc_char | (char_ - lit(_r1)))
            ;
        BOOST_SPIRIT_DEBUG_NODES((line)(column)(quoted));
    }
private:
    qi::rule<Iterator, csv_line(char, char),  Skipper> line;
    qi::rule<Iterator, csv_value(char, char)> column; // no-skip
    qi::rule<Iterator, csv_value(char)> text; // no-skip
    qi::rule<Iterator, qi::locals<char>, csv_value(char)> quoted; //no-skip
    qi::symbols<char const, char const> unesc_char;
};

}

#endif // MAPNIK_CVS_GRAMMAR_HPP
