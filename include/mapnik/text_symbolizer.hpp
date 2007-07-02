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

#ifndef TEXT_SYMBOLIZER_HPP
#define TEXT_SYMBOLIZER_HPP
// stl
#include <string>
// boost
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
// mapnik
#include <mapnik/color.hpp>
#include <mapnik/graphics.hpp> 

namespace mapnik
{
   enum label_placement_e {
      point_placement=1,
      line_placement=2
   };
        
   typedef boost::tuple<double,double> position;
    
   struct MAPNIK_DECL text_symbolizer
   {		
         text_symbolizer(std::string const& name,std::string const& face_name, 
                         unsigned size,Color const& fill);	
         text_symbolizer(text_symbolizer const& rhs);
         text_symbolizer& operator=(text_symbolizer const& rhs);
         std::string const& get_name() const;
         unsigned get_text_ratio() const; // target ratio for text bounding box in pixels
         void set_text_ratio(unsigned ratio);
         unsigned get_wrap_width() const; // width to wrap text at, or trigger ratio
         void set_wrap_width(unsigned ratio);
         unsigned get_label_spacing() const; // spacing between repeated labels on lines
         void set_label_spacing(unsigned spacing);
         unsigned get_label_position_tolerance() const; //distance the label can be moved on the line to fit, if 0 the default is used
         void set_label_position_tolerance(unsigned tolerance);
         bool get_force_odd_labels() const; // try render an odd amount of labels
         void set_force_odd_labels(bool force);
         double get_max_char_angle_delta() const; // maximum change in angle between adjacent characters
         void set_max_char_angle_delta(double angle);
         unsigned get_text_size() const;
         std::string const& get_face_name() const;
         Color const& get_fill() const;
         void set_halo_fill(Color const& fill);
         Color const& get_halo_fill() const;
         void set_halo_radius(unsigned radius);
         unsigned get_halo_radius() const;
         void set_label_placement(label_placement_e label_p);
         label_placement_e get_label_placement() const;
         void set_anchor(double x, double y);	
         position const& get_anchor() const;	
         void set_displacement(double x, double y);
         position const& get_displacement() const;
         void set_avoid_edges(bool avoid);
         bool get_avoid_edges() const;
         void set_minimum_distance(double distance);
         double get_minimum_distance() const;
         void set_allow_overlap(bool overlap);
         bool get_allow_overlap() const;
      private:
         std::string name_;
         std::string face_name_;
         unsigned size_;
         unsigned text_ratio_;
         unsigned wrap_width_;
         unsigned label_spacing_;
         unsigned label_position_tolerance_;
         bool force_odd_labels_;
         double max_char_angle_delta_;
         Color fill_;
         Color halo_fill_;
         unsigned halo_radius_;
         label_placement_e label_p_;
         position anchor_;
         position displacement_;
         bool avoid_edges_;
         double minimum_distance_;
         bool overlap_;
   };
}

#endif //TEXT_SYMBOLIZER_HPP
