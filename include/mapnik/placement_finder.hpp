/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
 * Copyright (C) 2006 10East Corp.
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

#ifndef __PLACEMENT_FINDER__
#define __PLACEMENT_FINDER__

#include <mapnik/ctrans.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/text_symbolizer.hpp>
#include <mapnik/shield_symbolizer.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/text_path.hpp>

#include <queue>

namespace mapnik
{
   typedef text_path placement_element;
   
   struct placement : boost::noncopyable
   { 
         placement(string_info & info_, 
                   //path_type & shape_path_,
                   shield_symbolizer const& sym);
         
         placement(string_info & info_, 
                   //path_type & shape_path_,
                   text_symbolizer const& sym);
         
         ~placement();
        
         //helpers
         //std::pair<double, double> get_position_at_distance(double target_distance);
         //double get_total_distance();

         string_info & info;
         //path_type & shape_path;
         //double total_distance_; //cache for distance
    
         position displacement_;
         label_placement_e label_placement;

         std::queue< Envelope<double> > envelopes;
    
         //output
         boost::ptr_vector<placement_element> placements;
         
         int wrap_width;
         int text_ratio;

         int label_spacing; // distance between repeated labels on a single geometry
         unsigned label_position_tolerance; //distance the label can be moved on the line to fit, if 0 the default is used
         bool force_odd_labels; //Always try render an odd amount of labels

         double max_char_angle_delta;
         double minimum_distance;
         bool avoid_edges;
         bool has_dimensions;
         std::pair<double, double> dimensions;
   };
   
   template <typename DetectorT>
   class placement_finder : boost::noncopyable
   {
      public:
         //e is the dimensions of the map, buffer is the buffer used for collision detection.
         placement_finder(DetectorT & detector,Envelope<double> const& e);
         
         template <typename T>
         void find_placements(placement & p, T & path);
         
         void find_point_placement(placement & p, double, double);
         
         template <typename T>
         void find_placements_with_spacing(placement & p, T & path);
         
         void clear();
         
      private:
         template <typename T>
         bool build_path_follow(placement & p, double target_distance, T & path);
         
         template <typename T>
         bool build_path_horizontal(placement & p, double target_distance, T & path);
         
         void get_ideal_placements(placement & p, double distance, std::vector<double>&);
         
         void update_detector(placement & p);
         
         DetectorT & detector_;
         Envelope<double> const& dimensions_;
   };  
}

#endif

