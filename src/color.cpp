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

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/karma.hpp>
#pragma GCC diagnostic pop

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

namespace  {

static std::uint8_t multiply(std::uint8_t c, std::uint8_t a)
{
    std::uint32_t t = c * a + 128;
    return std::uint8_t(((t >> 8) + t) >> 8);
}

}

bool color::premultiply()
{
    if (premultiplied_) return false;
    if (alpha_ != 255)
    {
        red_ = multiply(red_, alpha_);
        green_ = multiply(green_, alpha_);
        blue_ = multiply(blue_, alpha_);
    }
    premultiplied_ = true;
    return true;
}

bool color::demultiply()
{
    if (!premultiplied_) return false;
    if (alpha_ < 255)
    {
        if (alpha_ == 0)
        {
            red_ = green_ = blue_ = 0;
        }
        else
        {
            std::uint32_t r = (std::uint32_t(red_) * 255) / alpha_;
            std::uint32_t g = (std::uint32_t(green_) * 255) / alpha_;
            std::uint32_t b = (std::uint32_t(blue_) * 255) / alpha_;
            red_ = (r > 255) ? 255 : r;
            green_ = (g > 255) ? 255 : g;
            blue_ = (b > 255) ? 255 : b;
        }
    }
    premultiplied_ = false;
    return true;
}
}
