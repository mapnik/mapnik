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

#include <boost/format.hpp>
#include "config.hpp"
#include <sstream>

namespace mapnik {

    class MAPNIK_DECL Color
    {
    private:
        unsigned int rgba_;
    public:
        Color()
            :rgba_(0xffffffff) {}

        Color(int red,int green,int blue,int alpha=0xff)
            : rgba_((alpha&0xff) << 24 | (blue&0xff) << 16 | (green&0xff) << 8 | red&0xff) {}
	
        explicit Color(int rgba)
            : rgba_(rgba) {}

        Color(const Color& rhs)
            : rgba_(rhs.rgba_) {}

        Color& operator=(const Color& rhs)
        {
            if (this==&rhs) return *this;
            rgba_=rhs.rgba_;
            return *this;
        }
        inline unsigned int blue() const
        {
            return (rgba_>>16)&0xff;
        }
        inline unsigned int green() const
        {
            return (rgba_>>8)&0xff;
        }
        inline unsigned int red() const
        {
            return rgba_&0xff;
        }
	
        inline void set_red(unsigned int r)
        {
            rgba_ = (rgba_ & 0xffffff00) | (r&0xff);
        }
	
        inline void set_green(unsigned int g)
        {
            rgba_ = (rgba_ & 0xffff00ff) | ((g&0xff) << 8);
        }
	
        inline void set_blue(unsigned int b)
        {
            rgba_ = (rgba_ & 0xff00ffff) | ((b&0xff) << 16);
        }

        inline unsigned int alpha() const
        {
            return (rgba_>>24)&0xff;
        }
	
        inline unsigned int rgba() const
        {
            return rgba_;
        }
        inline void set_bgr(unsigned bgr)
        {
            rgba_ = (rgba_ & 0xff000000) | (bgr & 0xffffff);
        }
        inline bool operator==(Color const& other) const
        {
            return rgba_ == other.rgba_;
        }

        inline std::string to_string() const
        {
            std::stringstream ss;
            ss << "rgb (" << red() << ","  << green() << ","  << blue() <<")";
            return ss.str();
        }
        inline std::string to_hex_string() const
        {
            std::stringstream ss;
            ss << boost::format("#%1$02x%2$02x%3$02x") % red() % green() % blue();
            return ss.str();
        }
    };    
}

#endif //COLOR_HPP
