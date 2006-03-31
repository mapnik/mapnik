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

//$Id$

#include "text_symbolizer.hpp"

namespace mapnik
{
    text_symbolizer::text_symbolizer(std::string const& name,unsigned size,Color const& fill)
	: name_(name),
	  size_(size),
	  fill_(fill),
	  halo_fill_(Color(255,255,255)),
	  halo_radius_(0),
	  label_p_(point_placement),
	  anchor_(0.0,0.5),
	  displacement_(0.0,0.0)  {}
           
    text_symbolizer::text_symbolizer(text_symbolizer const& rhs)
	: name_(rhs.name_),
	  size_(rhs.size_),
	  fill_(rhs.fill_),
	  halo_fill_(rhs.halo_fill_),
	  halo_radius_(rhs.halo_radius_),
	  label_p_(rhs.label_p_),
	  anchor_(rhs.anchor_),
	  displacement_(rhs.displacement_) {}

    text_symbolizer& text_symbolizer::operator=(text_symbolizer const& other)
    {
	if (this == &other)
	    return *this;
	name_ = other.name_;
	size_ = other.size_;
	fill_ = other.fill_;
	halo_fill_ = other.halo_fill_;
	label_p_ = other.label_p_;
	anchor_ = other.anchor_;
	displacement_ = other.displacement_; 
	return *this;
    } 

    std::string const&  text_symbolizer::get_name() const
    {
	return name_;
    }
    
    unsigned  text_symbolizer::get_text_size() const
    {
	return size_;
    }
	
    Color const&  text_symbolizer::get_fill() const
    {
	return fill_;
    }
	
    void  text_symbolizer::set_halo_fill(Color const& fill)
    {
	halo_fill_ = fill;
    }

    Color const&  text_symbolizer::get_halo_fill() const
    {
	return halo_fill_;
    }
	
    void  text_symbolizer::set_halo_radius(unsigned radius)
    {
	halo_radius_ = radius;
    }
	
    unsigned  text_symbolizer::get_halo_radius() const
    {
	return halo_radius_;
    }
	
    void  text_symbolizer::set_label_placement(label_placement_e label_p)
    {
	label_p_ = label_p;
    }
	
    label_placement_e  text_symbolizer::get_label_placement() const
    {
	return label_p_;
    }

    void  text_symbolizer::set_anchor(double x, double y)
    {
	anchor_ = boost::make_tuple(x,y);
    }
    
    position const& text_symbolizer::get_anchor () const
    {
	return anchor_;
    }
    void  text_symbolizer::set_displacement(double x, double y)
    {
	displacement_ = boost::make_tuple(x,y);
    }
    
    position const&  text_symbolizer::get_displacement() const
    {
	return displacement_;
    }
}
