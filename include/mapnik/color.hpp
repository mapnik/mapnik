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
//boost
#include <boost/format.hpp>
// stl
#include <sstream>

namespace mapnik {
     
    class MAPNIK_DECL color
    {
    private:
        unsigned int abgr_;
    public:
        color()
            :abgr_(0xffffffff) {}

        color(int red,int green,int blue,int alpha=0xff)
            : abgr_(((alpha&0xff) << 24) | 
                    ((blue&0xff) << 16)  | 
                    ((green&0xff) << 8)  | 
                     (red&0xff)) {}
        
        color( std::string const& css_string);
        
        explicit color(int rgba)
            : abgr_(rgba) {}

        color(const color& rhs)
            : abgr_(rhs.abgr_) {}

        color& operator=(const color& rhs)
        {
            if (this==&rhs) return *this;
            abgr_=rhs.abgr_;
            return *this;
        }
        inline unsigned int red() const
        {
            return abgr_&0xff;
        }
        inline unsigned int green() const
        {
            return (abgr_>>8)&0xff;
        }
        inline unsigned int blue() const
        {
            return (abgr_>>16)&0xff;
        }
        inline unsigned int alpha() const
        {
            return (abgr_>>24)&0xff;
        }	
        inline void set_red(unsigned int r)
        {
            abgr_ = (abgr_ & 0xffffff00) | (r&0xff);
        }
        inline void set_green(unsigned int g)
        {
            abgr_ = (abgr_ & 0xffff00ff) | ((g&0xff) << 8);
        }
        inline void set_blue(unsigned int b)
        {
            abgr_ = (abgr_ & 0xff00ffff) | ((b&0xff) << 16);
        }
        inline void set_alpha(unsigned int a)
        {
            abgr_ = (abgr_ & 0x00ffffff) | ((a&0xff) << 24);
        }

        inline unsigned int rgba() const
        {
            return abgr_;
        }
        inline void set_bgr(unsigned bgr)
        {
            abgr_ = (abgr_ & 0xff000000) | (bgr & 0xffffff);
        }
        inline bool operator==(color const& other) const
        {
            return abgr_ == other.abgr_;
        }
        
        inline bool operator!=(color const& other) const
        {
            return abgr_ != other.abgr_;
        }
        
        std::string to_string() const;
        
        std::string to_hex_string() const;
    };    
}

#endif //COLOR_HPP
