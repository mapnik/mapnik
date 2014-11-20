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

#include <mapnik/parse_transform.hpp>
#include <mapnik/transform_expression_grammar.hpp>

// stl
#include <string>
#include <stdexcept>

namespace mapnik {

transform_list_ptr parse_transform(std::string const& str, std::string const& encoding)
{
    static const transform_expression_grammar<std::string::const_iterator> g;
    transform_list_ptr tl = std::make_shared<transform_list>();
    std::string::const_iterator itr = str.begin();
    std::string::const_iterator end = str.end();
    bool r = qi::phrase_parse(itr, end, g, space_type(), *tl);
    if (r && itr == end)
    {
        return tl;
    }
    else
    {
        throw std::runtime_error("Failed to parse transform: \"" + str + "\"");
    }
}


} // namespace mapnik
