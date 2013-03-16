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
#include <boost/cstdint.hpp>
#include <boost/operators.hpp>

// stl
#include <sstream>

namespace mapnik {

class MAPNIK_DECL color
    : boost::equality_comparable<color>
{
private:
    boost::uint8_t red_;
    boost::uint8_t green_;
    boost::uint8_t blue_;
    boost::uint8_t alpha_;

public:
    color()
      : red_(0xff),
        green_(0xff),
        blue_(0xff),
        alpha_(0xff)
        {}

    color(unsigned red, unsigned green, unsigned blue, unsigned alpha = 0xff)
      : red_(red),
        green_(green),
        blue_(blue),
        alpha_(alpha)
        {}

    color(const color& rhs)
      : red_(rhs.red_),
        green_(rhs.green_),
        blue_(rhs.blue_),
        alpha_(rhs.alpha_)
        {}

    color( std::string const& str);

    std::string to_string() const;
    std::string to_hex_string() const;
    void premultiply();
    void demultiply();

    color& operator=(color const& rhs)
    {
        if (this==&rhs) 
            return *this;
        
        red_   = rhs.red_;
        green_ = rhs.green_;
        blue_  = rhs.blue_;
        alpha_ = rhs.alpha_;
        
        return *this;
    }

    inline bool operator==(color const& rhs) const
    {
        return (red_== rhs.red()) &&
               (green_ == rhs.green()) &&
               (blue_  == rhs.blue()) &&
               (alpha_ == rhs.alpha());
    }

    inline unsigned red() const
    {
        return red_;
    }

    inline unsigned green() const
    {
        return green_;
    }
    inline unsigned blue() const
    {
        return blue_;
    }
    inline unsigned alpha() const
    {
        return alpha_;
    }

    inline void set_red(unsigned red)
    {
        red_ = red;
    }
    inline void set_green(unsigned green)
    {
        green_ = green;
    }

    inline void set_blue(unsigned blue)
    {
        blue_ = blue;
    }
    inline void set_alpha(unsigned alpha)
    {
        alpha_ = alpha;
    }

    inline unsigned rgba() const
    {
#ifdef MAPNIK_BIG_ENDIAN
        return (alpha_) | (blue_ << 8) | (green_ << 16) | (red_ << 24) ;
#else
        return (alpha_ << 24) | (blue_ << 16) | (green_ << 8) | (red_) ;
#endif
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

}

#endif // MAPNIK_COLOR_HPP
