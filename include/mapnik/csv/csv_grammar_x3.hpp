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

#ifndef MAPNIK_CSV_GRAMMAR_X3_HPP
#define MAPNIK_CSV_GRAMMAR_X3_HPP

#include <mapnik/csv/csv_types.hpp>
#include <boost/spirit/home/x3.hpp>
#include <iostream>
namespace mapnik {

namespace x3 = boost::spirit::x3;

struct csv_white_space_skipper : x3::parser<csv_white_space_skipper>
{
    using attribute_type = x3::unused_type;
    static bool const has_attribute = false;

    template<typename Iterator, typename Context, typename Attribute>
    bool parse(Iterator& first, Iterator const& last, Context const& context, x3::unused_type, Attribute&) const
    {
        x3::skip_over(first, last, context);
        if (first != last && *first == ' ')
        {
            while (++first != last && *first == ' ')
                ;
            return true;
        }
        return false;
    }
};

auto static const csv_white_space = csv_white_space_skipper{};

namespace grammar {

struct separator_tag;
struct quote_tag;

struct csv_line_class;
using csv_line_grammar_type = x3::rule<csv_line_class, csv_line>;

csv_line_grammar_type const line = "csv-line";

BOOST_SPIRIT_DECLARE(csv_line_grammar_type);

} // namespace grammar
} // namespace mapnik

#endif // MAPNIK_CSV_GRAMMAR_X3_HPP
