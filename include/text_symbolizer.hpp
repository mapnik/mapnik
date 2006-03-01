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

#ifndef TEXT_SYMBOLIZER_HPP
#define TEXT_SYMBOLIZER_HPP

namespace mapnik
{
    enum label_placement_e {
	point_placement=1,
	line_placement=2
    };
        
    struct text_symbolizer
    {		
	text_symbolizer(std::string const& name,unsigned size,Color const& fill)
	    : name_(name),
	      size_(size),
	      fill_(fill),
	      halo_fill_(Color(255,255,255)),
	      halo_radius_(0),
	      label_p_(point_placement) {}
	
	text_symbolizer(text_symbolizer const& rhs)
	    : name_(rhs.name_),
	      size_(rhs.size_),
	      fill_(rhs.fill_),
	      halo_fill_(rhs.halo_fill_),
	      halo_radius_(rhs.halo_radius_),
	      label_p_(rhs.label_p_) {}
	
	~text_symbolizer()
	{
	    //
	}
	std::string const& get_name() const
	{
	    return name_;
	}
	
	unsigned get_text_size() const
	{
	    return size_;
	}
	
	Color const& get_fill() const
	{
	    return fill_;
	}
	
	void set_halo_fill(Color const& fill)
	{
	    halo_fill_ = fill;
	}

	Color const& get_halo_fill() const
	{
	    return halo_fill_;
	}
	
	void set_halo_radius(unsigned radius)
	{
	    halo_radius_ = radius;
	}
	
	unsigned get_halo_radius() const
	{
	    return halo_radius_;
	}
	
	void set_label_placement(label_placement_e label_p)
	{
	    label_p_ = label_p;
	}
	
	label_placement_e get_label_placement() const
	{
	    return label_p_;
	}
	
    private:
	std::string name_;
	unsigned size_;
	Color fill_;
	Color halo_fill_;
	unsigned halo_radius_;
	label_placement_e label_p_;
    };
}

#endif //TEXT_SYMBOLIZER_HPP
