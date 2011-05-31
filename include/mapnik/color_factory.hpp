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

//$Id$

#ifndef MAPNIK_COLOR_FACTORY_HPP
#define MAPNIK_COLOR_FACTORY_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/color.hpp>
#include <mapnik/config_error.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/version.hpp>

// boost 1.41 -> 1.44 compatibility, to be removed in mapnik 2.1 (dane)
#if BOOST_VERSION >= 104500
#include <mapnik/css_color_grammar.hpp>

namespace mapnik {    
   
class MAPNIK_DECL color_factory : boost::noncopyable
{
public:
    
    static void init_from_string(color & c, char const* css_color)
    {   
        typedef char const* iterator_type;
        typedef mapnik::css_color_grammar<iterator_type> css_color_grammar; 

        css_color_grammar g;
        iterator_type first = css_color;
        iterator_type last =  css_color + std::strlen(css_color);
        bool result =
            boost::spirit::qi::phrase_parse(first,
                                            last,
                                            g,
                                            boost::spirit::ascii::space,
                                            c);
        if (!result) 
        {
            throw config_error(std::string("Failed to parse color value: ") +
                               "Expected a CSS color, but got '" + css_color + "'");
        }
    }    
    
    static color from_string(char const* css_color)
    {   
        color c;
        init_from_string(c,css_color);
        return c;
    }
};
}

#else
#include <mapnik/css_color_grammar_deprecated.hpp>

namespace mapnik {    
   
class MAPNIK_DECL color_factory : boost::noncopyable
{
public:
    
    static void init_from_string(color & c, char const* css_color)
    {   
        typedef char const* iterator_type;
        typedef mapnik::css_color_grammar<iterator_type> css_color_grammar; 

        css_color_grammar g;
        iterator_type first = css_color;
        iterator_type last =  css_color + std::strlen(css_color);
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
                               "Expected a CSS color, but got '" + css_color + "'");
        }
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

#endif

#endif //MAPNIK_COLOR_FACTORY_HPP
