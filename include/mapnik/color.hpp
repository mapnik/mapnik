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

#ifndef MAPNIK_COLOR_HPP
#define MAPNIK_COLOR_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/global.hpp>

//boost
#include <boost/operators.hpp>

// stl
#include <sstream>
#include <cstdint>

namespace mapnik {

class MAPNIK_DECL color
    : boost::equality_comparable<color>
{
private:
    std::uint8_t red_;
    std::uint8_t green_;
    std::uint8_t blue_;
    std::uint8_t alpha_;
    bool premultiplied_;
public:
    // default ctor
    color()
      : red_(0xff),
        green_(0xff),
        blue_(0xff),
        alpha_(0xff),
        premultiplied_(false)
        {}

    color(std::uint8_t _red, std::uint8_t _green, std::uint8_t _blue, std::uint8_t _alpha = 0xff, bool premultiplied = false)
      : red_(_red),
        green_(_green),
        blue_(_blue),
        alpha_(_alpha),
        premultiplied_(premultiplied)
        {}

    color(std::uint32_t _rgba, bool premultiplied = false)
      : red_(_rgba & 0xff),
        green_((_rgba >> 8) & 0xff),
        blue_((_rgba >> 16) & 0xff),
        alpha_((_rgba >> 24) & 0xff),
        premultiplied_(premultiplied) {}

    // copy ctor
    color(const color& rhs)
      : red_(rhs.red_),
        green_(rhs.green_),
        blue_(rhs.blue_),
        alpha_(rhs.alpha_),
        premultiplied_(rhs.premultiplied_)
        {}

    // move ctor
    color(color && rhs)
        : red_(std::move(rhs.red_)),
        green_(std::move(rhs.green_)),
        blue_(std::move(rhs.blue_)),
        alpha_(std::move(rhs.alpha_)),
        premultiplied_(std::move(rhs.premultiplied_)) {}

    color( std::string const& str, bool premultiplied = false);

    std::string to_string() const;
    std::string to_hex_string() const;
    bool premultiply();
    bool demultiply();

    color& operator=(color rhs)
    {
        swap(rhs);
        return *this;
    }

    inline bool operator==(color const& rhs) const
    {
        return (red_== rhs.red()) &&
               (green_ == rhs.green()) &&
               (blue_  == rhs.blue()) &&
               (alpha_ == rhs.alpha());
    }

    inline std::uint8_t red() const
    {
        return red_;
    }

    inline std::uint8_t green() const
    {
        return green_;
    }

    inline std::uint8_t blue() const
    {
        return blue_;
    }

    inline std::uint8_t alpha() const
    {
        return alpha_;
    }

    inline void set_red(std::uint8_t _red)
    {
        red_ = _red;
    }

    inline void set_green(std::uint8_t _green)
    {
        green_ = _green;
    }

    inline void set_blue(std::uint8_t _blue)
    {
        blue_ = _blue;
    }
    inline void set_alpha(std::uint8_t _alpha)
    {
        alpha_ = _alpha;
    }
    inline bool get_premultiplied() const
    {
        return premultiplied_;
    }
    inline void set_premultiplied(bool status)
    {
        premultiplied_ = status;
    }

    inline unsigned rgba() const
    {
        return static_cast<unsigned>((alpha_ << 24) | (blue_ << 16) | (green_ << 8) | (red_)) ;
    }
private:
    void swap(color & rhs)
    {
        std::swap(red_, rhs.red_);
        std::swap(green_,rhs.green_);
        std::swap(blue_,rhs.blue_);
        std::swap(alpha_,rhs.alpha_);
    }
};

template <typename charT, typename traits>
std::basic_ostream<charT, traits> &
operator << ( std::basic_ostream<charT, traits> & s, mapnik::color const& c )
{
    s << c.to_string();
    return s;
}

// hash
inline std::size_t hash_value(color const& c)
{
    return c.rgba();
}

}

#endif // MAPNIK_COLOR_HPP
