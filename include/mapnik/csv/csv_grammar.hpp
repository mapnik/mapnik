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

#ifndef MAPNIK_CVS_GRAMMAR_HPP
#define MAPNIK_CVS_GRAMMAR_HPP

//#define BOOST_SPIRIT_DEBUG

#include <boost/spirit/include/qi.hpp>

namespace mapnik {

namespace qi = boost::spirit::qi;
using column  = std::string;
using columns = std::vector<column>;
using csv_line = columns;
using csv_data = std::vector<csv_line>;

template <typename Iterator>
struct csv_line_grammar : qi::grammar<Iterator, csv_line(std::string const&), qi::blank_type>
{
    csv_line_grammar() : csv_line_grammar::base_type(line)
    {
        using namespace qi;
        qi::_r1_type _r1;
        qi::lit_type lit;
        qi::eol_type eol;
        qi::char_type char_;

        unesc_char.add
            ("\\a", '\a')
            ("\\b", '\b')
            ("\\f", '\f')
            ("\\n", '\n')
            ("\\r", '\r')
            ("\\t", '\t')
            ("\\v", '\v')
            ("\\\\",'\\')
            //("\\\'", '\')
            ("\\\"", '\"')
            ;

        line   = column(_r1) % char_(_r1)
            ;
        column = quoted | *(char_ - (lit(_r1) /*| eol*/))
            ;
        quoted = '"' >> *("\"\"" | unesc_char |  ~char_('"')) >> '"'
            ;

        //http://stackoverflow.com/questions/7436481/how-to-make-my-split-work-only-on-one-real-line-and-be-capable-to-skeep-quoted-p/7462539#7462539
        BOOST_SPIRIT_DEBUG_NODES((line)(column)(quoted));
    }
  private:
    qi::rule<Iterator, csv_line(std::string const&), qi::blank_type> line;
    qi::rule<Iterator, column(std::string const&)> column; // no-skip
    qi::rule<Iterator, std::string()> quoted;
    qi::symbols<char const, char const> unesc_char;
};

template <typename Iterator>
struct csv_file_grammar : qi::grammar<Iterator, csv_data(std::string const&), qi::blank_type>
{
    csv_file_grammar() : csv_file_grammar::base_type(start)
    {
        using namespace qi;
        qi::eol_type eol;
        qi::_r1_type _r1;
        start  = -line(_r1) % eol
            ;
        BOOST_SPIRIT_DEBUG_NODES((start));
    }
  private:
    qi::rule<Iterator, csv_data(std::string const&), qi::blank_type> start;
    csv_line_grammar<Iterator> line;
};


}

#endif // MAPNIK_CVS_GRAMMAR_HPP
