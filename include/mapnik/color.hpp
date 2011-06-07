/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id: color.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef COLOR_HPP
#define COLOR_HPP

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
        :  red_(red),
        green_(green),
        blue_(blue),
        alpha_(alpha)
        {}
    
    color( std::string const& css_string);
        
    color(const color& rhs)
        : red_(rhs.red_),
        green_(rhs.green_),
        blue_(rhs.blue_),
        alpha_(rhs.alpha_) 
        {}

    color& operator=(const color& rhs)
    {
        if (this==&rhs) return *this;
        red_=rhs.red_;
        green_=rhs.green_;
        blue_=rhs.blue_;
        alpha_=rhs.alpha_;
        return *this;
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
    
    inline bool operator==(color const& rhs) const
    {
        return (red_== rhs.red()) &&
            (green_ == rhs.green()) &&
            (blue_  == rhs.blue()) &&
            (alpha_ == rhs.alpha());
        
    }
    
    std::string to_string() const;        
    std::string to_hex_string() const;
};    


}

#endif //COLOR_HPP
