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

// mapnik
#include <mapnik/color.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/css_color_grammar.hpp>

// boost
#include <boost/format.hpp>
#include <boost/version.hpp>

// stl
#include <sstream>


namespace mapnik {

color parse_color(std::string const& str)
{
    css_color_grammar<std::string::const_iterator> g;
    return parse_color(str, g);
}

color parse_color(std::string const& str,
                  css_color_grammar<std::string::const_iterator> const& g)
{
    color c;

    std::string::const_iterator first = str.begin();
    std::string::const_iterator last =  str.end();
    
    // boost 1.41 -> 1.44 compatibility, to be removed in mapnik 2.1 (dane)
#if BOOST_VERSION >= 104500
    bool result = boost::spirit::qi::phrase_parse(first, last, g,
                                                  boost::spirit::ascii::space,
                                                  c);
#else
    mapnik::css css_;
    bool result = boost::spirit::qi::phrase_parse(first, last, g,
                                                  boost::spirit::ascii::space, 
                                                  css_);
    c.set_red(css_.r);
    c.set_green(css_.g);
    c.set_blue(css_.b);
    c.set_alpha(css_.a);
    
#endif

    if (result && (first == last))
        return c;
    else
        throw config_error( "Failed to parse color: \"" + str + "\"" );
}

}
