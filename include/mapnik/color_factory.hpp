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

#ifndef MAPNIK_COLOR_FACTORY_HPP
#define MAPNIK_COLOR_FACTORY_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/color.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/css_color_grammar.hpp>

// boost
#include <boost/utility.hpp>

namespace mapnik {    
   
class MAPNIK_DECL color_factory : boost::noncopyable
{
public:
    
    static void init_from_string(color & c, char const* css_color)
    {   
	
	typedef std::string::const_iterator iterator_type;
	typedef mapnik::css_color_grammar<iterator_type> css_color_grammar; 
	std::string str(css_color);
	
	css_color_grammar g;
	iterator_type first = str.begin();
	iterator_type last =  str.end();
	mapnik::css css_;
	bool result =
	    boost::spirit::qi::phrase_parse(first,
					    last,
					    g,
					    boost::spirit::ascii::space,
					    css_);
	if (!result) 
	{
	    throw config_error(std::string("Failed to parse color value: ") +
			       "Expected a color, but got '" + css_color + "'");
	}
	// TODO: adapt mapnik::color into boost::fusion sequence
	c.set_red(css_.r);
	c.set_green(css_.g);
	c.set_blue(css_.b);
	c.set_alpha(css_.a);
    }    
    
    static color from_string(char const* css_color)
    {   
	color c;
	init_from_string(c,css_color);
	return c;
    }
};
}

#endif //MAPNIK_COLOR_FACTORY_HPP
