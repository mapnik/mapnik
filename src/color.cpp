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

// boost
#include <boost/format.hpp>
#include <boost/version.hpp>

// stl
#include <sstream>

// boost 1.41 -> 1.44 compatibility, to be removed in mapnik 2.1 (dane)
#if BOOST_VERSION >= 104500
#include <mapnik/css_color_grammar.hpp>
#else
#include <mapnik/css_color_grammar_deprecated.hpp>
#endif


namespace mapnik {

color::color( std::string const& css_string)
    : red_(0),
      green_(0),
      blue_(0),
      alpha_(0xff)
{
    color_factory::init_from_string(*this,css_string);
}

std::string color::to_string() const
{
    std::stringstream ss;
    if (alpha_ == 255)
    {
        ss << "rgb("
           << red()   << ","
           << green() << ","
           << blue()  << ")";
    }
    else
    {
        ss << "rgba("
           << red()   << ","
           << green() << ","
           << blue()  << ","
           << alpha()/255.0 << ")";
    }
    return ss.str();
}

std::string color::to_hex_string() const
{
    if (alpha_ == 255 )
    {
        return (boost::format("#%1$02x%2$02x%3$02x")
                % red()
                % green()
                % blue() ).str();
    }
    else
    {
        return (boost::format("#%1$02x%2$02x%3$02x%4$02x")
                % red()
                % green()
                % blue()
                % alpha()).str();
    }
}


/****************************************************************************/
void color_factory::init_from_string(color & c, std::string const& css_color)
{
    typedef std::string::const_iterator iterator_type;
    typedef mapnik::css_color_grammar<iterator_type> css_color_grammar;

    css_color_grammar g;
    iterator_type first = css_color.begin();
    iterator_type last =  css_color.end();
    // boost 1.41 -> 1.44 compatibility, to be removed in mapnik 2.1 (dane)
#if BOOST_VERSION >= 104500
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
#else
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
#endif
}

bool color_factory::parse_from_string(color & c, std::string const& css_color,
                                      mapnik::css_color_grammar<std::string::const_iterator> const& g)
{
    std::string::const_iterator first = css_color.begin();
    std::string::const_iterator last =  css_color.end();
    // boost 1.41 -> 1.44 compatibility, to be removed in mapnik 2.1 (dane)
#if BOOST_VERSION >= 104500
    bool result =
        boost::spirit::qi::phrase_parse(first,
                                        last,
                                        g,
                                        boost::spirit::ascii::space,
                                        c);
    return result && (first == last);
#else
    mapnik::css css_;
    bool result =
        boost::spirit::qi::phrase_parse(first,
                                        last,
                                        g,
                                        boost::spirit::ascii::space,
                                        css_);
    if (result && (first == last))
    {
        c.set_red(css_.r);
        c.set_green(css_.g);
        c.set_blue(css_.b);
        c.set_alpha(css_.a);
        return true;
    }
    return false;
#endif
}


color color_factory::from_string(std::string const& css_color)
{
    color c;
    init_from_string(c, css_color);
    return c;
}

}

