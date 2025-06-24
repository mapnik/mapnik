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

// mapnik
#include <mapnik/image_filter_types.hpp>
#include <mapnik/image_filter_grammar_x3.hpp>
// stl
#include <vector>

namespace mapnik {

namespace filter {

bool parse_image_filters(std::string const& str, std::vector<filter_type>& image_filters)
{
    auto const& grammar = mapnik::image_filter::start;
    auto itr = str.begin();
    auto end = str.end();

    boost::spirit::x3::ascii::space_type space;
    bool r = false;
    try
    {
        r = boost::spirit::x3::phrase_parse(itr, end, grammar, space, image_filters);
    }
    catch (...)
    {
        image_filters.clear();
        r = false;
    }
    return r && itr == end;
}

} // namespace filter
} // namespace mapnik
