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

// agg
#include "agg_color_rgba.h"

// boost
#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/phoenix_statement.hpp>
// stl
#include <sstream>

namespace mapnik {

color::color(std::string const& str)
{
    *this = parse_color(str);
}

std::string color::to_string() const
{
    namespace karma = boost::spirit::karma;
    boost::spirit::karma::_1_type _1;
    boost::spirit::karma::eps_type eps;
    boost::spirit::karma::double_type double_;
    boost::spirit::karma::string_type kstring;
    boost::spirit::karma::uint_generator<uint8_t,10> color_generator;
    std::string str;
    std::back_insert_iterator<std::string> sink(str);
    karma::generate(sink,
                    // begin grammar
                    kstring[ phoenix::if_(alpha()==255) [_1="rgb("].else_[_1="rgba("]]
                    << color_generator[_1 = red()] << ','
                    << color_generator[_1 = green()] << ','
                    << color_generator[_1 = blue()]
                    << kstring[ phoenix::if_(alpha()==255) [_1 = ')'].else_[_1 =',']]
                    << eps(alpha()<255) << double_ [_1 = alpha()/255.0]
                    << ')'
                    // end grammar
        );
    return str;
}

std::string color::to_hex_string() const
{
    namespace karma = boost::spirit::karma;
    boost::spirit::karma::_1_type _1;
    boost::spirit::karma::hex_type hex;
    boost::spirit::karma::eps_type eps;
    boost::spirit::karma::right_align_type right_align;
    std::string str;
    std::back_insert_iterator<std::string> sink(str);
    karma::generate(sink,
                    // begin grammar
                    '#'
                    << right_align(2,'0')[hex[_1 = red()]]
                    << right_align(2,'0')[hex[_1 = green()]]
                    << right_align(2,'0')[hex[_1 = blue()]]
                    << eps(alpha() < 255) <<  right_align(2,'0')[hex [_1 = alpha()]]
                    // end grammar
        );
    return str;
}

void color::premultiply()
{
    agg::rgba8 pre_c = agg::rgba8(red_,green_,blue_,alpha_);
    pre_c.premultiply();
    red_ = pre_c.r;
    green_ = pre_c.g;
    blue_ = pre_c.b;
}

void color::demultiply()
{
    // note: this darkens too much: https://github.com/mapnik/mapnik/issues/1519
    agg::rgba8 pre_c = agg::rgba8(red_,green_,blue_,alpha_);
    pre_c.demultiply();
    red_ = pre_c.r;
    green_ = pre_c.g;
    blue_ = pre_c.b;
}

}
