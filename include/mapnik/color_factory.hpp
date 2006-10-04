/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id$

#ifndef COLOR_FACTORY_HPP
#define COLOR_FACTORY_HPP

#include <mapnik/config.hpp>
#include <mapnik/css_color_parser.hpp>

using namespace boost::spirit;

namespace mapnik {    
    class MAPNIK_DECL color_factory
    {
    public:
        static Color from_string(char const* css_color)
        {   
            Color color;
            actions<Color> a(color);
            css_color_grammar<actions<Color> > grammar(a);
            parse_info<> info = parse(css_color, grammar, space_p);
            if (info.full) return color;
            return Color(0,0,0);	
        }    
    private:
        color_factory();
        color_factory(color_factory const&);
        color_factory& operator=(color_factory const&);
    };
}

#endif //COLOR_FACTORY_HPP
