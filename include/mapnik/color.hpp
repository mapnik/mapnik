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
#include <boost/format.hpp>
#include <boost/cstdint.hpp>

// stl
#include <sstream>

namespace mapnik {
     
    class MAPNIK_DECL color
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

        color(int red,int green,int blue,int alpha=0xff)
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
        
        inline unsigned int green() const
        {
            return green_;
        }
        inline unsigned int blue() const
        {
            return blue_;
        }
        inline unsigned int alpha() const
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
        inline void set_alpha(unsigned int alpha)
        {
            alpha_ = alpha;
        }

        inline unsigned int rgba() const
        {
#ifdef MAPNIK_BIG_ENDIAN
            return (alpha_) | (blue_ << 8) | (green_ << 16) | (red_ << 24) ;
#else
            return (alpha_ << 24) | (blue_ << 16) | (green_ << 8) | (red_) ;
#endif
        }
        
        inline bool operator==(color const& other) const
        {
            return rgba() == other.rgba();
        }
        
        inline bool operator!=(color const& other) const
        {
            return rgba() != other.rgba();
        }
        
        std::string to_string() const;
        
        std::string to_hex_string() const;
    };    
}

#endif //COLOR_HPP
