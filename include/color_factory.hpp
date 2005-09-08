/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id$

#ifndef COLOR_FACTORY_HPP
#define COLOR_FACTORY_HPP

#include "css_color_parser.hpp"

namespace mapnik
{
    using namespace boost::spirit;
    class color_factory
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
