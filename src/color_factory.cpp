/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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
#include <mapnik/color.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/css/css_color_grammar_x3.hpp>

namespace mapnik {

color parse_color(std::string const& str)
{
    // TODO - early return for @color?
    auto const& grammar = mapnik::css_color_grammar::css_color;
    color c;
    std::string::const_iterator first = str.begin();
    std::string::const_iterator last = str.end();
    using namespace boost::spirit::x3::ascii;

    bool result = boost::spirit::x3::phrase_parse(first, last, grammar, space, c);
    if (result && (first == last))
    {
        return c;
    }
    else
    {
        throw config_error("Failed to parse color: \"" + str + "\"");
    }
}

} // namespace mapnik
