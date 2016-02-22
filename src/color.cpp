/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/karma.hpp>
#pragma GCC diagnostic pop

// stl
#include <sstream>

namespace mapnik {

color::color(std::string const& str, bool premultiplied)
{
    *this = parse_color(str);
    premultiplied_ = premultiplied;
}

std::string color::to_string() const
{
    namespace karma = boost::spirit::karma;
    boost::spirit::karma::eps_type eps;
    boost::spirit::karma::double_type double_;
    boost::spirit::karma::uint_generator<uint8_t,10> color_;
    std::string str;
    std::back_insert_iterator<std::string> sink(str);
    karma::generate(sink, eps(alpha() < 255)
                    // begin grammar
                    << "rgba("
                    << color_(red()) << ','
                    << color_(green()) << ','
                    << color_(blue()) << ','
                    << double_(alpha()/255.0) << ')'
                    |
                    "rgb("
                    << color_(red()) << ','
                    << color_(green()) << ','
                    << color_(blue()) << ')'
                    // end grammar
        );
    return str;
}

std::string color::to_hex_string() const
{
    namespace karma = boost::spirit::karma;
    boost::spirit::karma::hex_type hex;
    boost::spirit::karma::eps_type eps;
    boost::spirit::karma::right_align_type right_align;
    std::string str;
    std::back_insert_iterator<std::string> sink(str);
    karma::generate(sink,
                    // begin grammar
                    '#'
                    << right_align(2,'0')[hex(red())]
                    << right_align(2,'0')[hex(green())]
                    << right_align(2,'0')[hex(blue())]
                    << eps(alpha() < 255) << right_align(2,'0')[hex(alpha())]
                    // end grammar
        );
    return str;
}

bool color::premultiply()
{
    if (premultiplied_) return false;
    agg::rgba8 pre_c = agg::rgba8(red_,green_,blue_,alpha_);
    pre_c.premultiply();
    red_ = pre_c.r;
    green_ = pre_c.g;
    blue_ = pre_c.b;
    premultiplied_ = true;
    return true;
}

bool color::demultiply()
{
    if (!premultiplied_) return false;
    agg::rgba8 pre_c = agg::rgba8(red_,green_,blue_,alpha_);
    pre_c.demultiply();
    red_ = pre_c.r;
    green_ = pre_c.g;
    blue_ = pre_c.b;
    premultiplied_ = false;
    return true;
}

}
