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


#include <iostream>

#include <mapnik/text_symbolizer.hpp>

namespace mapnik
{
    text_symbolizer::text_symbolizer(std::string const& name, std::string const& face_name, unsigned size,Color const& fill)
	: name_(name),
          face_name_(face_name),
	  size_(size),
          text_ratio_(0),
          wrap_width_(0),
          label_spacing_(0),
          label_position_tolerance_(0),
          force_odd_labels_(false),
          max_char_angle_delta_(0),
	  fill_(fill),
	  halo_fill_(Color(255,255,255)),
	  halo_radius_(0),
	  label_p_(point_placement),
	  anchor_(0.0,0.5),
	  displacement_(0.0,0.0),
    avoid_edges_(false) {}
           
    text_symbolizer::text_symbolizer(text_symbolizer const& rhs)
        : name_(rhs.name_),
          face_name_(rhs.face_name_),
          size_(rhs.size_),
          text_ratio_(rhs.text_ratio_),
          wrap_width_(rhs.wrap_width_),
          label_spacing_(rhs.label_spacing_),
          label_position_tolerance_(rhs.label_position_tolerance_),
          force_odd_labels_(rhs.force_odd_labels_),
          max_char_angle_delta_(rhs.max_char_angle_delta_),
          fill_(rhs.fill_),
          halo_fill_(rhs.halo_fill_),
          halo_radius_(rhs.halo_radius_),
          label_p_(rhs.label_p_),
          anchor_(rhs.anchor_),
          displacement_(rhs.displacement_),
          avoid_edges_(rhs.avoid_edges_) {}

    text_symbolizer& text_symbolizer::operator=(text_symbolizer const& other)
    {
        if (this == &other)
            return *this;
        name_ = other.name_;
        face_name_ = other.face_name_;
        size_ = other.size_;
        text_ratio_ = other.text_ratio_;
        wrap_width_ = other.wrap_width_;
        label_spacing_ = other.label_spacing_;
        label_position_tolerance_ = other.label_position_tolerance_;
        force_odd_labels_ = other.force_odd_labels_;
        max_char_angle_delta_ = other.max_char_angle_delta_;
        fill_ = other.fill_;
        halo_fill_ = other.halo_fill_;
        halo_radius_ = other.halo_radius_;
        label_p_ = other.label_p_;
        anchor_ = other.anchor_;
        displacement_ = other.displacement_; 
        avoid_edges_ = other.avoid_edges_;
        
        return *this;
    } 

    std::string const&  text_symbolizer::get_name() const
    {
        return name_;
    }
    
    std::string const&  text_symbolizer::get_face_name() const
    {
        return face_name_;
    }
    
    unsigned  text_symbolizer::get_text_ratio() const
    {
    return text_ratio_;
    }

    void  text_symbolizer::set_text_ratio(unsigned ratio) 
    {
        text_ratio_ = ratio;
    }

    unsigned  text_symbolizer::get_wrap_width() const
    {
    return wrap_width_;
    }

    void  text_symbolizer::set_wrap_width(unsigned width) 
    {
        wrap_width_ = width;
    }    

    unsigned  text_symbolizer::get_label_spacing() const
    {
        return label_spacing_;
    }

    void  text_symbolizer::set_label_spacing(unsigned spacing) 
    {
        label_spacing_ = spacing;
    }

    unsigned  text_symbolizer::get_label_position_tolerance() const
    {
        return label_position_tolerance_;
    }

    void  text_symbolizer::set_label_position_tolerance(unsigned tolerance)
    {
        label_position_tolerance_ = tolerance;
    }

    bool  text_symbolizer::get_force_odd_labels() const
    {
        return force_odd_labels_;
    }

    void  text_symbolizer::set_force_odd_labels(bool force) 
    {
        force_odd_labels_ = force;
    }

    double text_symbolizer::get_max_char_angle_delta() const
    {
        return max_char_angle_delta_;
    }

    void text_symbolizer::set_max_char_angle_delta(double angle) 
    {
        max_char_angle_delta_ = angle;
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
    
    position const& text_symbolizer::get_displacement() const
    {
        return displacement_;
    }
    
    bool text_symbolizer::get_avoid_edges() const
    {
        return avoid_edges_;
    }
    
    void text_symbolizer::set_avoid_edges(bool avoid)
    {
        avoid_edges_ = avoid;
    }
}
