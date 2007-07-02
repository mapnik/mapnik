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

#include <queue>

#include <mapnik/ctrans.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/text_symbolizer.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/text_path.hpp>

namespace mapnik
{

   struct placement_element
   {
         double starting_x;
         double starting_y;
    
         text_path path;
   };

   struct placement
   {
         typedef  coord_transform2<CoordTransform,geometry_type> path_type;
         
         template <typename SymbolizerT>
         placement(string_info *info_, 
                   CoordTransform *ctrans_, 
                   const proj_transform *proj_trans_, 
                   geometry_ptr geom_,
                   SymbolizerT sym);
         
         ~placement();
        
         //helpers
         std::pair<double, double> get_position_at_distance(double target_distance);
         double get_total_distance();
         void clear_envelopes();
    
         unsigned path_size() const;

         string_info *info;
    
         CoordTransform *ctrans;
         const proj_transform *proj_trans;
         geometry_ptr geom;
         path_type shape_path;

         double total_distance_; //cache for distance
    
         position displacement_;
         label_placement_e label_placement;

         std::queue< Envelope<double> > envelopes;
    
         //output
         std::vector<placement_element> placements;

         // caching output
         placement_element current_placement;
    
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
         //e is the dimensions of the map, buffer is the buffer used for collission detection.
         placement_finder(DetectorT & detector,Envelope<double> const& e);
         void find_placements(placement *p);
         void clear();
         
      protected:
         bool find_placement_follow(placement *p);
         bool find_placement_horizontal(placement *p);
         bool build_path_follow(placement *p, double target_distance);
         bool build_path_horizontal(placement *p, double target_distance);
         std::vector<double> get_ideal_placements(placement *p);
         void update_detector(placement *p);
         DetectorT & detector_;
         Envelope<double> dimensions_;
   };  
}

#endif

