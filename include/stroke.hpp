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

//$Id$

#ifndef STROKE_HPP
#define STROKE_HPP

#include "color.hpp"
#include <vector>

namespace mapnik
{
    using std::pair;
    using std::vector;
    typedef vector<pair<float,float> > dash_array;
    
    enum line_cap_e
    {
	BUTT_CAP,
	SQUARE_CAP,
	ROUND_CAP
    }; 
    
    enum line_join_e
    {
	MITER_JOIN,
	MITER_REVERT_JOIN,
	ROUND_JOIN,
	BEVEL_JOIN
    };
    
    class stroke
    {	
	Color c_;
	float width_;
	float opacity_; // 0.0 - 1.0
	line_cap_e  line_cap_;
	line_join_e line_join_;
	dash_array dash_;	
    public:
	stroke() 
	    : c_(),
	      width_(1.0),
	      opacity_(1.0),
	      line_cap_(BUTT_CAP),
	      line_join_(MITER_JOIN),
	      dash_() {}
	
	stroke(Color const& c, float width=1.0)
	    : c_(c),
	      width_(width),
	      opacity_(1.0),
	      line_cap_(BUTT_CAP),
	      line_join_(MITER_JOIN),
	      dash_() {}

	stroke(stroke const& other)
	    : c_(other.c_),
	      width_(other.width_),
	      opacity_(other.opacity_),
	      line_cap_(other.line_cap_),
	      line_join_(other.line_join_),
	      dash_(other.dash_) {}
	    
	stroke& operator=(const stroke& rhs)
	{
	    stroke tmp(rhs);
	    swap(tmp);
	    return *this;
	}

	void set_color(const Color& c) 
	{
	    c_=c;
	}
	
	Color const& get_color() const
	{
	    return c_;
	}
	
	float get_width() const
	{
	    return width_;
	}
	void set_width(float w)
	{
	    width_=w;
	}

	void set_opacity(float opacity)
	{    
	    if (opacity > 1.0) opacity_=1.0;
	    else if (opacity < 0.0) opacity_=0.0;
	    else opacity_=opacity;
	}

	float get_opacity() const 
	{
	    return opacity_;
	}
	
	void set_line_cap(line_cap_e line_cap)
	{
	    line_cap_=line_cap;
	}

	line_cap_e get_line_cap() const 
	{
	    return line_cap_;
	}
	
	void set_line_join(line_join_e line_join) 
	{
	    line_join_=line_join;
	}
	
	line_join_e get_line_join() const 
	{
	    return line_join_;
	}
	
	void add_dash(float dash,float gap)
	{
	    dash_.push_back(std::make_pair(dash,gap));
	}
	bool has_dash() const 
	{
	    return dash_.size();
	}
	
	dash_array const& get_dash_array() const
	{
	    return  dash_;
	}
	
    private:
	void swap(const stroke& other) throw()
	{
	    c_=other.c_;
	    width_=other.width_;
	    opacity_=other.opacity_;
	    line_cap_=other.line_cap_;
	    line_join_=other.line_join_;
	    dash_ = other.dash_;
	}
    };
}

#endif //STROKE_HPP
