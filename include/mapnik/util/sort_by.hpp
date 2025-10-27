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


#ifndef MAPNIK_SORT_BY_HPP
#define MAPNIK_SORT_BY_HPP

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/std_pair.hpp>

namespace mapnik {

using sort_by_type = std::pair<std::string, bool>;

inline bool parse_sort_by(std::string const& str, sort_by_type & result)
{
    namespace x3 = boost::spirit::x3;
    auto itr = str.begin();
    auto end = str.end();
    auto apply_sort_by = [&](auto const& ctx) {
        result.first = _attr(ctx);
    };
    auto apply_desc = [&](auto const& ctx) {
        result.second = true;
    };
    if (!x3::phrase_parse(itr, end,
                          x3::no_skip[(+x3::char_("a-zA-Z_0-9-"))][apply_sort_by]
                          > -(x3::no_case[x3::lit("DESC")][apply_desc] | x3::no_case[x3::lit("ASC")]),
                          // ASC is a default
                          x3::space
            ) || (itr != end))
    {
        return false;
    }
    return true;
}

} // namespace mapnik
#endif //  MAPNIK_SORT_BY_HPP
