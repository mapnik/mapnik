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

#ifndef MAPNIK_CSV_GRAMMAR_HPP
#define MAPNIK_CSV_GRAMMAR_HPP

#include <mapnik/csv/csv_types.hpp>
#include <boost/spirit/include/qi.hpp>

namespace mapnik {

namespace qi = boost::spirit::qi;

struct csv_white_space_skipper : qi::primitive_parser<csv_white_space_skipper>
{
    template <typename Context, typename Iterator>
    struct attribute
    {
        typedef qi::unused_type type;
    };

    template <typename Iterator, typename Context
      , typename Skipper, typename Attribute>
    bool parse(Iterator& first, Iterator const& last
      , Context& /*context*/, Skipper const& skipper
      , Attribute& /*attr*/) const
    {
        qi::skip_over(first, last, skipper);
        if (first != last && *first == ' ')
        {
            while (++first != last && *first == ' ')
                ;
            return true;
        }
        return false;
    }

    template <typename Context>
    qi::info what(Context& /*context*/) const
    {
        return qi::info("csv_white_space_skipper");
    }
};


template <typename Iterator, typename Skipper = csv_white_space_skipper>
struct csv_line_grammar : qi::grammar<Iterator, csv_line(char, char), Skipper>
{
    csv_line_grammar();
private:
    qi::rule<Iterator, csv_line(char, char),  Skipper> line;
    qi::rule<Iterator, csv_value(char, char)> column; // no-skip
    qi::rule<Iterator, csv_value(char)> text; // no-skip
    qi::rule<Iterator, csv_value(char)> quoted; // no-skip
    qi::symbols<char const, char const> unesc_char;
};

}

#endif // MAPNIK_CSV_GRAMMAR_HPP
