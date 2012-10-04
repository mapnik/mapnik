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
#include <boost/format.hpp>

// stl
#include <sstream>

namespace mapnik {

color::color(std::string const& str)
{
    *this = parse_color(str);
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

