/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#include <mapnik/parse_path.hpp>
#include <mapnik/path_expression_grammar.hpp>

#include <boost/make_shared.hpp>

namespace mapnik {

path_expression_ptr parse_path(std::string const & str)
{
    path_expression_ptr path =  boost::make_shared<path_expression>();
    path_expression_grammar<std::string::const_iterator> g;

    std::string::const_iterator itr = str.begin();
    std::string::const_iterator end = str.end();
    bool r = qi::phrase_parse(itr, end, g, space, *path);
    if (r  && itr == end)
    {
        return path;
    }
    else
    {
        throw std::runtime_error("Failed to parse path expression");
    }
}

bool parse_path_from_string(path_expression_ptr const& path,
                                           std::string const & str,
                                           path_expression_grammar<std::string::const_iterator> const& g)
{
    std::string::const_iterator itr = str.begin();
    std::string::const_iterator end = str.end();
    bool r = qi::phrase_parse(itr, end, g, space, *path);
    return (r && itr==end);
}

}
