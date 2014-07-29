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

public:
    // default ctor
    color()
      : red_(0xff),
        green_(0xff),
        blue_(0xff),
        alpha_(0xff)
        {}

    color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha = 0xff)
      : red_(red),
        green_(green),
        blue_(blue),
        alpha_(alpha)
        {}

    // copy ctor
    color(const color& rhs)
      : red_(rhs.red_),
        green_(rhs.green_),
        blue_(rhs.blue_),
        alpha_(rhs.alpha_)
        {}

    // move ctor
    color(color && rhs)
        : red_(std::move(rhs.red_)),
        green_(std::move(rhs.green_)),
        blue_(std::move(rhs.blue_)),
        alpha_(std::move(rhs.alpha_)) {}

    color( std::string const& str);

    std::string to_string() const;
    std::string to_hex_string() const;
    void premultiply();
    void demultiply();

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

    inline void set_red(std::uint8_t red)
    {
        red_ = red;
    }

    inline void set_green(std::uint8_t green)
    {
        green_ = green;
    }

    inline void set_blue(std::uint8_t blue)
    {
        blue_ = blue;
    }
    inline void set_alpha(std::uint8_t alpha)
    {
        alpha_ = alpha;
    }

    inline unsigned rgba() const
    {
#ifdef MAPNIK_BIG_ENDIAN
        return static_cast<unsigned>((alpha_) | (blue_ << 8) | (green_ << 16) | (red_ << 24)) ;
#else
        return static_cast<unsigned>((alpha_ << 24) | (blue_ << 16) | (green_ << 8) | (red_)) ;
#endif
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
    std::string hex_string( c.to_string() );
    s << hex_string;
    return s;
}

// hash
inline std::size_t hash_value(color const& c)
{
    return c.rgba();
}

}

#endif // MAPNIK_COLOR_HPP
