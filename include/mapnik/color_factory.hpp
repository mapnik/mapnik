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

#ifndef MAPNIK_COLOR_FACTORY_HPP
#define MAPNIK_COLOR_FACTORY_HPP

// mapnik
#include <mapnik/config.hpp>

// boost
#include <boost/utility.hpp>

namespace mapnik {

class color;

template <typename Iterator> struct css_color_grammar;
class MAPNIK_DECL color_factory : boost::noncopyable
{
public:

    static void init_from_string(color & c, std::string const& css_color);

    static bool parse_from_string(color & c, std::string const& css_color,
                                  mapnik::css_color_grammar<std::string::const_iterator> const& g);

    static color from_string(std::string const& css_color);
};
}

#endif // MAPNIK_COLOR_FACTORY_HPP
