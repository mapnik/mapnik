/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id: color.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef COLOR_HPP
#define COLOR_HPP

namespace mapnik {

    class Color
    {
    private:
	int rgba_;
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
	inline int blue() const
	{
	    return (rgba_>>16)&0xff;
	}
	inline int green() const
	{
	    return (rgba_>>8)&0xff;
	}
	inline int red() const
	{
	    return rgba_&0xff;
	}

	inline int alpha() const
	{
	    return (rgba_>>24)&0xff;
	}
	
	inline int rgba() const
	{
	    return rgba_;
	}
	static const Color White;
	static const Color Black;
	static const Color Gray;
	static const Color Red;
	static const Color Green;
	static const Color Blue;
	static const Color Yellow;
    };    
}

#endif //COLOR_HPP
