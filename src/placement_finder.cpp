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

//mapnik
#include <mapnik/placement_finder.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/text_path.hpp>

// agg
#include "agg_path_length.h"
#include "agg_conv_clip_polyline.h"

// boost
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/tuple/tuple.hpp>

//stl
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace mapnik
{
   placement::placement(string_info & info_, 
                        shield_symbolizer const& sym)
      : info(info_), 
        displacement_(sym.get_displacement()),
        label_placement(sym.get_label_placement()), 
        wrap_width(sym.get_wrap_width()), 
        text_ratio(sym.get_text_ratio()), 
        label_spacing(sym.get_label_spacing()), 
        label_position_tolerance(sym.get_label_position_tolerance()), 
        force_odd_labels(sym.get_force_odd_labels()), 
        max_char_angle_delta(sym.get_max_char_angle_delta()),
        minimum_distance(sym.get_minimum_distance()),
        avoid_edges(sym.get_avoid_edges()),
        has_dimensions(true), 
        dimensions(std::make_pair(sym.get_image()->width(),
                                  sym.get_image()->height()))
   {
   }
   
   placement::placement(string_info & info_,
                        text_symbolizer const& sym)
      : info(info_), 
        displacement_(sym.get_displacement()),
        label_placement(sym.get_label_placement()), 
        wrap_width(sym.get_wrap_width()), 
        text_ratio(sym.get_text_ratio()), 
        label_spacing(sym.get_label_spacing()), 
        label_position_tolerance(sym.get_label_position_tolerance()), 
        force_odd_labels(sym.get_force_odd_labels()), 
        max_char_angle_delta(sym.get_max_char_angle_delta()),
        minimum_distance(sym.get_minimum_distance()),
        avoid_edges(sym.get_avoid_edges()),
        has_dimensions(false),
        dimensions()
   {
   }
   placement::~placement()
   {
   }
   
   template<typename T>
   std::pair<double, double> get_position_at_distance(double target_distance, T & shape_path)
   {
      double x1 = 0.0;
      double y1 = 0.0;
      double x2 = 0.0;
      double y2 = 0.0;
      double distance = 0.0;
      bool first = true;
      unsigned cmd;
      double x = 0.0;
      double y = 0.0;
      shape_path.rewind(0);
      while (!agg::is_stop(cmd = shape_path.vertex(&x2,&y2)))
      {
         if (first || agg::is_move_to(cmd))
         {
            first = false;
         }
         else
         {
            double dx = x2-x1;
            double dy = y2-y1;
            
            double segment_length = ::sqrt(dx*dx + dy*dy);
            distance +=segment_length;
            
            if (distance > target_distance)
            {
               x = x2 - dx * (distance - target_distance)/segment_length;
               y = y2 - dy * (distance - target_distance)/segment_length;
               break;
            }
         }
         x1 = x2;
         y1 = y2;
      }
      return std::pair<double, double>(x, y);
   }
   
   template<typename T>
   double get_total_distance(T & shape_path)
   {
      return agg::path_length(shape_path);
   }
   
   template <typename DetectorT>
   placement_finder<DetectorT>::placement_finder(DetectorT & detector,Envelope<double> const& e)
      : detector_(detector),
        dimensions_(e)
   {
   }

   template <typename DetectorT>
   void placement_finder<DetectorT>::get_ideal_placements(placement & p, double distance, std::vector<double> & ideal_label_distances)
   {
      std::pair<double, double> string_dimensions = p.info.get_dimensions();
      double string_width = string_dimensions.first;
    
      if (p.label_placement == LINE_PLACEMENT && string_width > distance)
      {
         //Empty!
         return ;
      }
      
      int num_labels = 0;
      if (p.label_spacing && p.label_placement == LINE_PLACEMENT)
      {
         num_labels = static_cast<int> (floor(distance / (p.label_spacing + string_width)));
      }
      else if (p.label_spacing && p.label_placement == POINT_PLACEMENT)
      {
         num_labels = static_cast<int> (floor(distance / p.label_spacing));
      }
      
      if (p.force_odd_labels && num_labels%2 == 0)
         num_labels--;
      if (num_labels <= 0)
         num_labels = 1;


      double ideal_spacing = distance/num_labels;
      
      double middle; //try draw text centered
      if (p.label_placement == LINE_PLACEMENT)
        middle = (distance / 2.0) - (string_width/2.0);
      else //  (p.label_placement == point_placement)
        middle = distance / 2.0;
		
      if (num_labels % 2) //odd amount of labels
      {
         for (int a = 0; a < (num_labels+1)/2; a++)
         {
            ideal_label_distances.push_back(middle - (a*ideal_spacing));
	
            if (a != 0)
               ideal_label_distances.push_back(middle + (a*ideal_spacing));
         }
      }
      else //even amount of labels
      {
         for (int a = 0; a < num_labels/2; a++)
         {
            ideal_label_distances.push_back(middle - (ideal_spacing/2.0) - (a*ideal_spacing));
            ideal_label_distances.push_back(middle + (ideal_spacing/2.0) + (a*ideal_spacing));
         }
      }
      
      if (p.label_position_tolerance == 0)
      {
         p.label_position_tolerance = unsigned(ideal_spacing/2.0);
      }
   }
   
   template <typename DetectorT>
   template <typename T>
   void placement_finder<DetectorT>::find_placements(placement & p, T & shape_path)
   {
      double distance = get_total_distance<T>(shape_path);
      std::vector<double> ideal_label_distances;
      get_ideal_placements(p,distance,ideal_label_distances);
      std::vector<double>::const_iterator itr = ideal_label_distances.begin();
      std::vector<double>::const_iterator end = ideal_label_distances.end();
      for (; itr != end; ++itr)
      {
               
         if ((p.label_placement == LINE_PLACEMENT &&
              build_path_follow(p, *itr , shape_path ) ) ||
             (p.label_placement == POINT_PLACEMENT &&
              build_path_horizontal(p, *itr, shape_path)) )
         {
            update_detector(p);
            break;
         }
      }    
   }
   
   template <typename DetectorT>
   void placement_finder<DetectorT>::find_point_placement(placement & p, 
                                                          double label_x, double label_y)
   {
      double x, y;
      std::auto_ptr<placement_element> current_placement(new placement_element);
      
      std::pair<double, double> string_dimensions = p.info.get_dimensions();
      double string_width = string_dimensions.first;
      double string_height = string_dimensions.second;
      
      // check if we need to wrap the string
      double wrap_at = string_width + 1;
      if (p.wrap_width && string_width > p.wrap_width)
      {
         if (p.text_ratio)
            for (int i = 1; ((wrap_at = string_width/i)/(string_height*i)) > p.text_ratio && (string_width/i) > p.wrap_width; ++i);
         else
            wrap_at = p.wrap_width;
      }
      
      // work out where our line breaks need to be
      std::vector<int> line_breaks;
      std::vector<double> line_widths;
      if (wrap_at < string_width && p.info.num_characters() > 0)
      {
         int line_count=0; 
         int last_space = 0;
         string_width = 0;
         string_height = 0;
         double line_width = 0;
         double line_height = 0;
         double word_width = 0;
         double word_height = 0;
         for (unsigned int ii = 0; ii < p.info.num_characters(); ii++)
         {
            character_info ci;
            ci = p.info.at(ii);
                
            unsigned c = ci.character;
            word_width += ci.width;
            word_height = word_height > ci.height ? word_height : ci.height;
            ++line_count;
        
            if (c == ' ')
            {
               last_space = ii;
               line_width += word_width;
               line_height = line_height > word_height ? line_height : word_height;
               word_width = 0;
               word_height = 0;
            }
            if (line_width > 0 && line_width > wrap_at)
            {
               string_width = string_width > line_width ? string_width : line_width;
               string_height += line_height;
               line_breaks.push_back(last_space);
               line_widths.push_back(line_width);
               ii = last_space;
               line_count = 0;
               line_width = 0;
               line_height = 0;
               word_width = 0;
               word_height = 0;
            }
         }
         line_width += word_width;
         string_width = string_width > line_width ? string_width : line_width;
         line_breaks.push_back(p.info.num_characters() + 1);
         line_widths.push_back(line_width);
      }
      if (line_breaks.size() == 0)
      {
         line_breaks.push_back(p.info.num_characters() + 1);
         line_widths.push_back(string_width);
      }
        
      p.info.set_dimensions(string_width, string_height);
      
       
      current_placement->starting_x = label_x;
      current_placement->starting_y = label_y;
      
      current_placement->starting_x += boost::tuples::get<0>(p.displacement_);
      current_placement->starting_y += boost::tuples::get<1>(p.displacement_); 
      
      double line_height = 0;
      unsigned int line_number = 0;
      unsigned int index_to_wrap_at = line_breaks[line_number];
      double line_width = line_widths[line_number];
    
      x = -line_width/2.0 - 1.0;
      y = -string_height/2.0 + 1.0;
    
      for (unsigned i = 0; i < p.info.num_characters(); i++)
      {
         character_info ci;;
         ci = p.info.at(i);
            
         unsigned c = ci.character;
         if (i == index_to_wrap_at)
         {
            index_to_wrap_at = line_breaks[++line_number];
            line_width = line_widths[line_number];
            y -= line_height;
            x = -line_width/2.0;
            line_height = 0;
            continue;
         }
         else
         {
            current_placement->add_node(c, x, y, 0.0);
    
            Envelope<double> e;
            if (p.has_dimensions)
            {
               e.init(current_placement->starting_x - (p.dimensions.first/2.0), 
                      current_placement->starting_y - (p.dimensions.second/2.0), 
                      current_placement->starting_x + (p.dimensions.first/2.0), 
                      current_placement->starting_y + (p.dimensions.second/2.0));
            }
            else
            {
               e.init(current_placement->starting_x + x, 
                      current_placement->starting_y - y, 
                      current_placement->starting_x + x + ci.width, 
                      current_placement->starting_y - y - ci.height);
            }
                
            if (!dimensions_.intersects(e) || 
                !detector_.has_placement(e, p.info.get_string(), p.minimum_distance))
            {
               return;
            }
            
            if (p.avoid_edges && !dimensions_.contains(e)) return;
            
            p.envelopes.push(e);
         }
         x += ci.width;
         line_height = line_height > ci.height ? line_height : ci.height;
      }
      p.placements.push_back(current_placement.release());
      update_detector(p);
   }
   
   
   template <typename DetectorT>
   template <typename PathT>
   void placement_finder<DetectorT>::find_placements_with_spacing(placement & p, PathT & shape_path)
   {
      double new_x = 0.0;
      double new_y = 0.0;
      double old_x = 0.0;
      double old_y = 0.0;
      double x = 0.0;
      double y = 0.0;
      
      double next_char_x = 0.0;
      double next_char_y = 0.0;
      
      shape_path.rewind(0);
      unsigned cmd;
      bool first = true;
      double distance = 0.0;
      std::pair<double, double> string_dimensions = p.info.get_dimensions();
      
      double string_width = string_dimensions.first;
      double string_height = string_dimensions.second;
      double spacing = p.label_spacing;
      
      double angle = 0.0;
      int orientation = 0;
      double displacement = boost::tuples::get<1>(p.displacement_); // displace by dy
      double target_distance = spacing;
      
      //Calculate a target_distance that will place the labels centered evenly rather than offset from the start of the linestring
      const double total_distance = get_total_distance(shape_path);
      shape_path.rewind(0);
      
      if (total_distance < string_width) //Can't place any strings
        return;
      
      int num_labels = static_cast<int> (floor(total_distance / (spacing + string_width)));
      if (p.force_odd_labels && num_labels%2 == 0)
         num_labels--;
      if (num_labels <= 0)
         num_labels = 1;
      
      //Now we know how many labels we are going to place, recalculate the spacing so that they will get placed evenly
      spacing = (total_distance - (num_labels * string_width)) / num_labels;
      target_distance = spacing / 2;
      

      while (!agg::is_stop(cmd = shape_path.vertex(&new_x,&new_y))) //For each node in the shape
      {
         if (first || agg::is_move_to(cmd)) //Don't do any processing if it is the first node
         {
            first = false;
         }
         else 
         {
            //Add the length of this segment to the total we have saved up
            double dx = new_x - old_x;
            double dy = new_y - old_y;
            double segment_length = ::sqrt(dx*dx + dy*dy);
            distance += segment_length;
            
            //While we have enough distance to place text in
            while (distance > target_distance)
            {
               //Record details for the start of the string placement
               std::auto_ptr<placement_element> current_placement(new placement_element);
               current_placement->starting_x = new_x - dx*(distance - target_distance)/segment_length;
               current_placement->starting_y = new_y - dy*(distance - target_distance)/segment_length;
               angle = atan2(-dy, dx);
               orientation = (angle > 0.55*M_PI || angle < -0.45*M_PI) ? -1 : 1;
               distance -= target_distance; //Consume the spacing gap we have used up
               target_distance = spacing; //Need to reset the target_distance as it is spacing/2 for the first label.
               // now find the placement of each character starting from our initial segment
               // determined above
               double last_angle = angle; 
               bool status = true;
               for (unsigned i = 0; i < p.info.num_characters(); ++i)
               {
                  character_info ci;
                  unsigned c;
                  
                  // grab the next character according to the orientation
                  ci = orientation > 0 ? p.info.at(i) : p.info.at(p.info.num_characters() - i - 1);
                  c = ci.character;
    
                  double angle_delta = 0;
                  
                  // if the distance remaining in this segment is less than the character width
                  // move to the next segment
                  if (distance <= ci.width) 
                  {   
                     last_angle = angle;
                     while (distance <= ci.width) 
                     {
                        old_x = new_x;
                        old_y = new_y;
                        //Stop if we run off the end of the shape
                        if (agg::is_stop(shape_path.vertex(&new_x,&new_y))) 
                        {
                           status = false;
                           break;
                        }
                        dx = new_x - old_x;
                        dy = new_y - old_y;
                        
                        angle = atan2(-dy, dx );
                        distance += sqrt(dx*dx+dy*dy);
                     }
                     // since our rendering angle has changed then check against our
                     // max allowable angle change.
                     angle_delta = last_angle - angle;
                     // normalise between -180 and 180
                     while (angle_delta > M_PI)
                        angle_delta -= 2*M_PI;
                     while (angle_delta < -M_PI)
                        angle_delta += 2*M_PI;
                     if (p.max_char_angle_delta > 0 && 
                         fabs(angle_delta) > p.max_char_angle_delta*(M_PI/180))
                     {
                        status = false;
                     }
                  }
                  
                  Envelope<double> e;
                  if (p.has_dimensions)
                  {
                     e.init(x, y, x + p.dimensions.first, y + p.dimensions.second);
                  }
                  
                  double render_angle = angle;
                  
                  x = new_x - (distance)*cos(angle);
                  y = new_y + (distance)*sin(angle);
                  
                  //Center the text on the line, unless displacement != 0
                  if (displacement == 0.0) 
                  {
                     x -= (((double)string_height/2.0) - 1.0)*cos(render_angle+M_PI/2);
                     y += (((double)string_height/2.0) - 1.0)*sin(render_angle+M_PI/2);
                  } 
                  else if (displacement*orientation > 0.0) 
                  {
                     x -= ((fabs(displacement) - (double)string_height) + 1.0)*cos(render_angle+M_PI/2);
                     y += ((fabs(displacement) - (double)string_height) + 1.0)*sin(render_angle+M_PI/2);
                  } 
                  else 
                  { // displacement < 0
                    x -= ((fabs(displacement) + (double)string_height) - 1.0)*cos(render_angle+M_PI/2);
                    y += ((fabs(displacement) + (double)string_height) - 1.0)*sin(render_angle+M_PI/2);
                  }
                  
                  distance -= ci.width;
                  next_char_x = ci.width*cos(render_angle);
                  next_char_y = ci.width*sin(render_angle);

                  double render_x = x;
                  double render_y = y;

                  if (!p.has_dimensions)
                  {
                     // put four corners of the letter into envelope
                     e.init(render_x, render_y, render_x + ci.width*cos(render_angle), 
                            render_y - ci.width*sin(render_angle));
                     e.expand_to_include(render_x - ci.height*sin(render_angle), 
                                         render_y - ci.height*cos(render_angle));
                     e.expand_to_include(render_x + (ci.width*cos(render_angle) - ci.height*sin(render_angle)), 
                                        render_y - (ci.width*sin(render_angle) + ci.height*cos(render_angle)));
                  }
                  
                  if (!dimensions_.intersects(e) || 
                      !detector_.has_placement(e, p.info.get_string(), p.minimum_distance))
                  {
                     status = false;
                  }
                  
                  if (p.avoid_edges && !dimensions_.contains(e))
                  {
                     status = false;
                  }

                  p.envelopes.push(e);
                  
                  if (orientation < 0)
                  {
                     // rotate in place
                     render_x += ci.width*cos(render_angle) - (string_height-2)*sin(render_angle);
                     render_y -= ci.width*sin(render_angle) + (string_height-2)*cos(render_angle);
                     render_angle += M_PI;
                  }
                             
                  current_placement->add_node(c,render_x - current_placement->starting_x, 
                                              -render_y + current_placement->starting_y, 
                                              render_angle);
                  x += next_char_x;
                  y -= next_char_y;
               }
               
               if (status)
               {
                  p.placements.push_back(current_placement.release());
                  update_detector(p);
               }
            }
         }
         
         old_x = new_x;
         old_y = new_y;
      }
   }
   
   
   template <typename DetectorT>
   void placement_finder<DetectorT>::update_detector(placement & p)
   {
      while (!p.envelopes.empty())
      {
         Envelope<double> e = p.envelopes.front();
         detector_.insert(e, p.info.get_string());
         p.envelopes.pop();
      }
   }

   template <typename DetectorT>
   template <typename PathT>
   bool placement_finder<DetectorT>::build_path_follow(placement & p, 
                                                       double target_distance, 
                                                       PathT & shape_path)
   {
      double new_x = 0.0;
      double new_y = 0.0;
      double old_x = 0.0;
      double old_y = 0.0;
      double next_char_x = 0.0;
      double next_char_y = 0.0;
      
      double angle = 0.0;
      int orientation = 0;
      double displacement = boost::tuples::get<1>(p.displacement_); // displace by dy
  
      std::auto_ptr<placement_element> current_placement(new placement_element);
      
      double x = 0.0;
      double y = 0.0;
      
      double distance = 0.0;
      
      std::pair<double, double> string_dimensions = p.info.get_dimensions();
      double string_height = string_dimensions.second;
      
      // find the segment that our text should start on
      shape_path.rewind(0);
      
      unsigned cmd;
      bool first = true;
      while (!agg::is_stop(cmd = shape_path.vertex(&new_x,&new_y)))
      {
         if (first || agg::is_move_to(cmd))
         {
            first = false;
         }
         else
         {
            double dx = new_x - old_x;
            double dy = new_y - old_y;
            double segment_length = sqrt(dx*dx + dy*dy);
            distance += segment_length;
            if (distance > target_distance)
            {
               current_placement->starting_x = new_x - dx*(distance - target_distance)/segment_length;
               current_placement->starting_y = new_y - dy*(distance - target_distance)/segment_length;
               
               // angle text starts at and orientation
               angle = atan2(-dy, dx);
               orientation = (angle > 0.55*M_PI || angle < -0.45*M_PI) ? -1 : 1;
               
               distance -= target_distance;
               
               break;
            }
         }
         old_x = new_x;
         old_y = new_y;
      }
      
      // now find the placement of each character starting from our initial segment
      // determined above
      double last_angle = angle; 
      for (unsigned i = 0; i < p.info.num_characters(); ++i)
      {
         character_info ci;
         unsigned c;

         // grab the next character according to the orientation
         ci = orientation > 0 ? p.info.at(i) : p.info.at(p.info.num_characters() - i - 1);
         c = ci.character;
    
         double angle_delta = 0;

         // if the distance remaining in this segment is less than the character width
         // move to the next segment
         if (distance <= ci.width) 
         {
            last_angle = angle;
            while (distance <= ci.width) 
            {
               double dx, dy;
               old_x = new_x;
               old_y = new_y;
                    
               if (agg::is_stop(shape_path.vertex(&new_x,&new_y)))
                  return false;
               dx = new_x - old_x;
               dy = new_y - old_y;
               
               angle = atan2(-dy, dx );
               distance += sqrt(dx*dx+dy*dy);
            }
            // since our rendering angle has changed then check against our
            // max allowable angle change.
            angle_delta = last_angle - angle;
            // normalise between -180 and 180
            while (angle_delta > M_PI)
               angle_delta -= 2*M_PI;
            while (angle_delta < -M_PI)
               angle_delta += 2*M_PI;
            if (p.max_char_angle_delta > 0 && fabs(angle_delta) > p.max_char_angle_delta*(M_PI/180))
            {
               return false;
            }
         }
            
         Envelope<double> e;
         if (p.has_dimensions)
         {
            e.init(x, y, x + p.dimensions.first, y + p.dimensions.second);
         }

         double render_angle = angle;

         x = new_x - (distance)*cos(angle);
         y = new_y + (distance)*sin(angle);
         //Center the text on the line, unless displacement != 0
         if (displacement == 0.0) {
           x -= (((double)string_height/2.0) - 1.0)*cos(render_angle+M_PI/2);
           y += (((double)string_height/2.0) - 1.0)*sin(render_angle+M_PI/2);
         } else if (displacement*orientation > 0.0) {
           x -= ((fabs(displacement) - (double)string_height) + 1.0)*cos(render_angle+M_PI/2);
           y += ((fabs(displacement) - (double)string_height) + 1.0)*sin(render_angle+M_PI/2);
         } else { // displacement < 0
           x -= ((fabs(displacement) + (double)string_height) - 1.0)*cos(render_angle+M_PI/2);
           y += ((fabs(displacement) + (double)string_height) - 1.0)*sin(render_angle+M_PI/2);
         }
         distance -= ci.width;
         next_char_x = ci.width*cos(render_angle);
         next_char_y = ci.width*sin(render_angle);

         double render_x = x;
         double render_y = y;

         if (!p.has_dimensions)
         {
            // put four corners of the letter into envelope
            e.init(render_x, render_y, render_x + ci.width*cos(render_angle), render_y - ci.width*sin(render_angle));
            e.expand_to_include(render_x - ci.height*sin(render_angle), render_y - ci.height*cos(render_angle));
            e.expand_to_include(render_x + (ci.width*cos(render_angle) - ci.height*sin(render_angle)), 
                                render_y - (ci.width*sin(render_angle) + ci.height*cos(render_angle)));
         }
         
         if (!dimensions_.intersects(e) || 
             !detector_.has_placement(e, p.info.get_string(), p.minimum_distance))
         {
            return false;
         }
         
         if (p.avoid_edges && !dimensions_.contains(e))
         {
            return false;
         }
         
         p.envelopes.push(e);

         if (orientation < 0)
         {
            // rotate in place
            render_x += ci.width*cos(render_angle) - (string_height-2)*sin(render_angle);
            render_y -= ci.width*sin(render_angle) + (string_height-2)*cos(render_angle);
            render_angle += M_PI;
         }
        
        
         current_placement->add_node(c,render_x - current_placement->starting_x, 
                                     -render_y + current_placement->starting_y, 
                                     render_angle);
         x += next_char_x;
         y -= next_char_y;
      }
      
      p.placements.push_back(current_placement.release());
      
      return true;
   }

   template <typename DetectorT>
   template <typename PathT>
   bool placement_finder<DetectorT>::build_path_horizontal(placement & p, double target_distance, PathT & shape_path)
   {
      double x, y;
      std::auto_ptr<placement_element> current_placement(new placement_element);
      
      std::pair<double, double> string_dimensions = p.info.get_dimensions();
      double string_width = string_dimensions.first;
      double string_height = string_dimensions.second;
        
      // check if we need to wrap the string
      double wrap_at = string_width + 1;
      if (p.wrap_width && string_width > p.wrap_width)
      {
         if (p.text_ratio)
            for (int i = 1; ((wrap_at = string_width/i)/(string_height*i)) > p.text_ratio && (string_width/i) > p.wrap_width; ++i);
         else
            wrap_at = p.wrap_width;
      }
      
      // work out where our line breaks need to be
      std::vector<int> line_breaks;
      std::vector<double> line_widths;
      if (wrap_at < string_width && p.info.num_characters() > 0)
      {
         int line_count=0; 
         int last_space = 0;
         string_width = 0;
         string_height = 0;
         double line_width = 0;
         double line_height = 0;
         double word_width = 0;
         double word_height = 0;
         for (unsigned int ii = 0; ii < p.info.num_characters(); ii++)
         {
            character_info ci;
            ci = p.info.at(ii);
                
            unsigned c = ci.character;
            word_width += ci.width;
            word_height = word_height > ci.height ? word_height : ci.height;
            ++line_count;
        
            if (c == ' ')
            {
               last_space = ii;
               line_width += word_width;
               line_height = line_height > word_height ? line_height : word_height;
               word_width = 0;
               word_height = 0;
            }
            if (line_width > 0 && line_width > wrap_at)
            {
               string_width = string_width > line_width ? string_width : line_width;
               string_height += line_height;
               line_breaks.push_back(last_space);
               line_widths.push_back(line_width);
               ii = last_space;
               line_count = 0;
               line_width = 0;
               line_height = 0;
               word_width = 0;
               word_height = 0;
            }
         }
         line_width += word_width;
         string_width = string_width > line_width ? string_width : line_width;
         line_breaks.push_back(p.info.num_characters() + 1);
         line_widths.push_back(line_width);
      }
      if (line_breaks.size() == 0)
      {
         line_breaks.push_back(p.info.num_characters() + 1);
         line_widths.push_back(string_width);
      }
        
      p.info.set_dimensions(string_width, string_height);
      
      std::pair<double, double> starting_pos = 
         get_position_at_distance<PathT>(target_distance,shape_path);
      current_placement->starting_x = starting_pos.first;
      current_placement->starting_y = starting_pos.second;
      
      double line_height = 0;
      unsigned int line_number = 0;
      unsigned int index_to_wrap_at = line_breaks[line_number];
      double line_width = line_widths[line_number];
    
      x = -line_width/2.0 - 1.0;
      y = -string_height/2.0 + 1.0;
    
      for (unsigned i = 0; i < p.info.num_characters(); i++)
      {
         character_info ci;;
         ci = p.info.at(i);
            
         unsigned c = ci.character;
         if (i == index_to_wrap_at)
         {
            index_to_wrap_at = line_breaks[++line_number];
            line_width = line_widths[line_number];
            y -= line_height;
            x = -line_width/2.0;
            line_height = 0;
            continue;
         }
         else
         {
            current_placement->add_node(c, x, y, 0.0);
    
            Envelope<double> e;
            if (p.has_dimensions)
            {
               e.init(current_placement->starting_x - (p.dimensions.first/2.0), 
                      current_placement->starting_y - (p.dimensions.second/2.0), 
                      current_placement->starting_x + (p.dimensions.first/2.0), 
                      current_placement->starting_y + (p.dimensions.second/2.0));
            }
            else
            {
               e.init(current_placement->starting_x + x, 
                      current_placement->starting_y - y, 
                      current_placement->starting_x + x + ci.width, 
                      current_placement->starting_y - y - ci.height);
            }
                
            if (!dimensions_.intersects(e) || 
                !detector_.has_placement(e, p.info.get_string(), p.minimum_distance))
            {
               return false;
            }
            
            if (p.avoid_edges && !dimensions_.contains(e))
            {
               return false;
            }
            
            p.envelopes.push(e);
         }
         x += ci.width;
         line_height = line_height > ci.height ? line_height : ci.height;
      }
      p.placements.push_back(current_placement.release());
      
      return true;
   }
   
   template <typename DetectorT>
   void placement_finder<DetectorT>::clear()
   {
      detector_.clear();
   }
   
   typedef coord_transform2<CoordTransform,geometry2d> PathType;
   typedef label_collision_detector4 DetectorType;
   
   template class placement_finder<DetectorType>;
   template void placement_finder<DetectorType>::find_placements<PathType> (placement&, PathType & );
   template void placement_finder<DetectorType>::find_placements_with_spacing<PathType> (placement&, PathType & );
   
}  // namespace
