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
       shield_symbolizer const& sym, bool has_dimensions_)
      : info(info_),
        displacement_(sym.get_displacement()),
        label_placement(sym.get_label_placement()),
        wrap_width(sym.get_wrap_width()),
        wrap_before(sym.get_wrap_before()),
        wrap_char(sym.get_wrap_char()),
        text_ratio(sym.get_text_ratio()),
        label_spacing(sym.get_label_spacing()),
        label_position_tolerance(sym.get_label_position_tolerance()),
        force_odd_labels(sym.get_force_odd_labels()),
        max_char_angle_delta(sym.get_max_char_angle_delta()),
        minimum_distance(sym.get_minimum_distance()),
        avoid_edges(sym.get_avoid_edges()),
        has_dimensions(has_dimensions_),
        allow_overlap(false),
        dimensions(std::make_pair(sym.get_image()->width(),
                                  sym.get_image()->height())),
        text_size(sym.get_text_size())
   {
   }

   placement::placement(string_info & info_,
                        text_symbolizer const& sym)
      : info(info_),
        displacement_(sym.get_displacement()),
        label_placement(sym.get_label_placement()),
        wrap_width(sym.get_wrap_width()),
        wrap_before(sym.get_wrap_before()),
        wrap_char(sym.get_wrap_char()),
        text_ratio(sym.get_text_ratio()),
        label_spacing(sym.get_label_spacing()),
        label_position_tolerance(sym.get_label_position_tolerance()),
        force_odd_labels(sym.get_force_odd_labels()),
        max_char_angle_delta(sym.get_max_char_angle_delta()),
        minimum_distance(sym.get_minimum_distance()),
        avoid_edges(sym.get_avoid_edges()),
        has_dimensions(false),
        allow_overlap(sym.get_allow_overlap()),
        dimensions(),
        text_size(sym.get_text_size())
   {}

   placement::~placement() {}

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

            double segment_length = std::sqrt(dx*dx + dy*dy);
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
   placement_finder<DetectorT>::placement_finder(DetectorT & detector)
      : detector_(detector),
        dimensions_(detector_.extent())
   {
   }

   template <typename DetectorT>
   template <typename T>
   void placement_finder<DetectorT>::find_point_placements(placement & p, T & shape_path)
   {
      unsigned cmd;
      double new_x = 0.0;
      double new_y = 0.0;
      double old_x = 0.0;
      double old_y = 0.0;
      bool first = true;

      double total_distance = get_total_distance<T>(shape_path);
      shape_path.rewind(0);

      if (distance == 0) //Point data, not a line
      {
         double x, y;
         shape_path.vertex(&x,&y);
         find_point_placement(p, x, y);
         return;
      }

      int num_labels = 1;
      if (p.label_spacing > 0)
         num_labels = static_cast<int> (floor(total_distance / p.label_spacing));

      if (p.force_odd_labels && num_labels%2 == 0)
         num_labels--;
      if (num_labels <= 0)
         num_labels = 1;

      double distance = 0.0; // distance from last label
      double spacing = total_distance / num_labels;
      double target_distance = spacing / 2; // first label should be placed at half the spacing

      while (!agg::is_stop(cmd = shape_path.vertex(&new_x,&new_y))) //For each node in the shape
      {

         if (first || agg::is_move_to(cmd)) //Don't do any processing if it is the first node
         {
            first = false;
         }
         else
         {
            //Add the length of this segment to the total we have saved up
            double segment_length = std::sqrt(std::pow(old_x-new_x,2) + std::pow(old_y-new_y,2)); //Pythagoras
            distance += segment_length;

            //While we have enough distance to place text in
            while (distance > target_distance)
            {
               //Try place at the specified place
               double new_weight = (segment_length - (distance - target_distance))/segment_length;
               find_point_placement(p, old_x + (new_x-old_x)*new_weight, old_y + (new_y-old_y)*new_weight);

               distance -= target_distance; //Consume the spacing gap we have used up
               target_distance = spacing; //Need to reset the target_distance as it is spacing/2 for the first label.
            }
         }

         old_x = new_x;
         old_y = new_y;
      }

   }

   template <typename DetectorT>
   void placement_finder<DetectorT>::find_point_placement(placement & p,
                                                          double label_x,
                                                          double label_y,
                                                          vertical_alignment_e valign,
                                                          unsigned line_spacing,
                                                          unsigned character_spacing,
                                                          horizontal_alignment_e halign,
                                                          justify_alignment_e jalign)
   {
      double x, y;
      std::auto_ptr<placement_element> current_placement(new placement_element);

      std::pair<double, double> string_dimensions = p.info.get_dimensions();
      double string_width = string_dimensions.first + (character_spacing *(p.info.num_characters()-1));
      double string_height = string_dimensions.second;

      // use height of tallest character in the string for the 'line' spacing to obtain consistent line spacing
      double max_character_height = string_height;  // height of the tallest character in the string

      // check if we need to wrap the string
      double wrap_at = string_width + 1.0;
      if (p.wrap_width && string_width > p.wrap_width)
      {
         if (p.text_ratio)
            for (double i = 1.0; ((wrap_at = string_width/i)/(string_height*i)) > p.text_ratio && (string_width/i) > p.wrap_width; i += 1.0);
         else
            wrap_at = p.wrap_width;
      }

      // work out where our line breaks need to be and the resultant width to the 'wrapped' string
      std::vector<int> line_breaks;
      std::vector<double> line_widths;

      if (wrap_at < string_width && p.info.num_characters() > 0)
      {
         int last_wrap_char = 0;
         int last_wrap_char_width = 0;
         string_width = 0.0;
         string_height = 0.0;
         double line_width = 0.0;
         double word_width = 0.0;

         for (unsigned int ii = 0; ii < p.info.num_characters(); ii++)
         {
            character_info ci;
            ci = p.info.at(ii);

            double cwidth = ci.width + character_spacing;

            unsigned c = ci.character;
            word_width += cwidth;

            if (c == p.wrap_char)
            {
               last_wrap_char = ii;
               last_wrap_char_width = cwidth;
               line_width += word_width;
               word_width = 0.0;
            }

            // wrap text at first wrap_char after (default) the wrap width or immediately before the current word
            if (line_width > 0 && (((line_width - character_spacing) > wrap_at && !p.wrap_before) ||
                                   ((line_width + word_width - character_spacing) > wrap_at && p.wrap_before)) )
            {
               // Remove width of breaking space character since it is not rendered and the character_spacing for the last character on the line
               line_width -= (last_wrap_char_width + character_spacing);
               string_width = string_width > line_width ? string_width : line_width;
               string_height += max_character_height;
               line_breaks.push_back(last_wrap_char);
               line_widths.push_back(line_width);
               ii = last_wrap_char;
               line_width = 0.0;
               word_width = 0.0;
            }
         }
         line_width += (word_width - character_spacing);  // remove character_spacing from last character on the line
         string_width = string_width > line_width ? string_width : line_width;
         string_height += max_character_height;
         line_breaks.push_back(p.info.num_characters());
         line_widths.push_back(line_width);
      }
      if (line_breaks.size() == 0)
      {
         line_breaks.push_back(p.info.num_characters());
         line_widths.push_back(string_width);
      }
      int   total_lines = line_breaks.size();

      p.info.set_dimensions( string_width, (string_height + (line_spacing * (total_lines-1))) );

      // if needed, adjust for desired vertical alignment
      current_placement->starting_y = label_y;  // no adjustment, default is MIDDLE

      if (valign == TOP)
         current_placement->starting_y -= 0.5 * (string_height + (line_spacing * (total_lines-1)));  // move center up by 1/2 the total height

      else if (valign == BOTTOM)
         current_placement->starting_y += 0.5 * (string_height + (line_spacing * (total_lines-1)));  // move center down by the 1/2 the total height

      // correct placement for error, but BOTTOM does not need to be adjusted
      // (text rendering is at text_size, but line placement is by line_height (max_character_height),
      //  and the rendering adds the extra space below the characters)
      if (valign == TOP )
         current_placement->starting_y -= (p.text_size - max_character_height);  // move up by the error

      else if (valign == MIDDLE)
         current_placement->starting_y -= ((p.text_size - max_character_height) / 2.0); // move up by 1/2 the error

      // set horizontal position to middle of text
      current_placement->starting_x = label_x;  // no adjustment, default is MIDDLE

      if (halign == H_LEFT)
         current_placement->starting_x -= 0.5 * string_width;  // move center left by 1/2 the string width

      else if (halign == H_RIGHT)
         current_placement->starting_x += 0.5 * string_width;  // move center right by 1/2 the string width

      // adjust text envelope position by user's x-y displacement (dx, dy)
      current_placement->starting_x += boost::tuples::get<0>(p.displacement_);
      current_placement->starting_y += boost::tuples::get<1>(p.displacement_);

      // presets for first line
      unsigned int line_number = 0;
      unsigned int index_to_wrap_at = line_breaks[0];
      double line_width = line_widths[0];

      // set for upper left corner of text envelope for the first line, bottom left of first character
      x = -(line_width / 2.0);
      y = (0.5 * (string_height + (line_spacing * (total_lines-1)))) - max_character_height;

      // if needed, adjust for desired justification (J_MIDDLE is the default)
      if( jalign == J_LEFT )
         x = -(string_width / 2.0);

      else if (jalign == J_RIGHT)
         x = (string_width / 2.0) - line_width;

      // save each character rendering position and build envelope as go thru loop
      std::queue< Envelope<double> > c_envelopes;

      for (unsigned i = 0; i < p.info.num_characters(); i++)
      {
         character_info ci;
         ci = p.info.at(i);

         double cwidth = ci.width + character_spacing;

         unsigned c = ci.character;
         if (i == index_to_wrap_at)
         {
            index_to_wrap_at = line_breaks[++line_number];
            line_width = line_widths[line_number];

            y -= (max_character_height + line_spacing);  // move position down to line start

            // reset to begining of line position
            x = ((jalign == J_LEFT)? -(string_width / 2.0): ((jalign == J_RIGHT)? ((string_width /2.0) - line_width): -(line_width / 2.0)));
            continue;
         }
         else
         {
            // place the character relative to the center of the string envelope
            current_placement->add_node(c, x, y, 0.0);

            // compute the Bounding Box for each character and test for:
            // overlap, minimum distance or edge avoidance - exit if condition occurs
            Envelope<double> e;
            if (p.has_dimensions)
            {
               e.init(current_placement->starting_x - (p.dimensions.first/2.0),     // Top Left
                      current_placement->starting_y - (p.dimensions.second/2.0),

                      current_placement->starting_x + (p.dimensions.first/2.0),     // Bottom Right
                      current_placement->starting_y + (p.dimensions.second/2.0));
            }
            else
            {
               e.init(current_placement->starting_x + x,                    // Bottom Left
                      current_placement->starting_y - y,

                      current_placement->starting_x + x + ci.width,         // Top Right
                      current_placement->starting_y - y - max_character_height);
            }

            // if there is an overlap with existing envelopes, then exit - no placement
            if (!dimensions_.intersects(e) || (!p.allow_overlap && !detector_.has_point_placement(e,p.minimum_distance)))
               return;

            if (p.avoid_edges && !dimensions_.contains(e))
               return;

            c_envelopes.push(e);  // add character's envelope to temp storage
         }
         x += cwidth;  // move position to next character
      }

      // since there was no early exit, add the character envelopes to the placements' envelopes
      while( !c_envelopes.empty() )
      {
        p.envelopes.push( c_envelopes.front() );
        c_envelopes.pop();
      }

      p.placements.push_back(current_placement.release());
   }


   template <typename DetectorT>
   template <typename PathT>
   void placement_finder<DetectorT>::find_line_placements(placement & p, PathT & shape_path)
   {
      unsigned cmd;
      double new_x = 0.0;
      double new_y = 0.0;
      double old_x = 0.0;
      double old_y = 0.0;
      bool first = true;

      //Pre-Cache all the path_positions and path_distances
      //This stops the PathT from having to do multiple re-projections if we need to reposition ourself
      // and lets us know how many points are in the shape.
      std::vector<vertex2d> path_positions;
      std::vector<double> path_distances; // distance from node x-1 to node x
      double total_distance = 0;

      shape_path.rewind(0);
      while (!agg::is_stop(cmd = shape_path.vertex(&new_x,&new_y))) //For each node in the shape
      {
         if (!first && agg::is_line_to(cmd))
         {
            double dx = old_x - new_x;
            double dy = old_y - new_y;
            double distance = std::sqrt(dx*dx + dy*dy);
            total_distance += distance;
            path_distances.push_back(distance);
         }
         else
         {
            path_distances.push_back(0);
         }
         first = false;
         path_positions.push_back(vertex2d(new_x, new_y, cmd));
         old_x = new_x;
         old_y = new_y;
      }
      //Now path_positions is full and total_distance is correct
      //shape_path shouldn't be used from here

      double distance = 0.0;
      std::pair<double, double> string_dimensions = p.info.get_dimensions();

      double string_width = string_dimensions.first;

      double displacement = boost::tuples::get<1>(p.displacement_); // displace by dy

      //Calculate a target_distance that will place the labels centered evenly rather than offset from the start of the linestring
      if (total_distance < string_width) //Can't place any strings
        return;

      //If there is no spacing then just do one label, otherwise calculate how many there should be
      int num_labels = 1;
      if (p.label_spacing > 0)
         num_labels = static_cast<int> (floor(total_distance / (p.label_spacing + string_width)));

      if (p.force_odd_labels && num_labels%2 == 0)
         num_labels--;
      if (num_labels <= 0)
         num_labels = 1;

      //Now we know how many labels we are going to place, calculate the spacing so that they will get placed evenly
      double spacing = total_distance / num_labels;
      double target_distance = (spacing - string_width) / 2; // first label should be placed at half the spacing

      //Calculate or read out the tolerance
      double tolerance_delta, tolerance;
      if (p.label_position_tolerance > 0)
      {
         tolerance = p.label_position_tolerance;
         tolerance_delta = std::max ( 1.0, p.label_position_tolerance/100.0 );
      }
      else
      {
         tolerance = spacing/2.0;
         tolerance_delta = std::max ( 1.0, spacing/100.0 );
      }


      first = true;
      for (unsigned index = 0; index < path_positions.size(); index++) //For each node in the shape
      {
         cmd = path_positions[index].cmd;
         new_x = path_positions[index].x;
         new_y = path_positions[index].y;

         if (first || agg::is_move_to(cmd)) //Don't do any processing if it is the first node
         {
            first = false;
         }
         else
         {
            //Add the length of this segment to the total we have saved up
            double segment_length = path_distances[index];
            distance += segment_length;

            //While we have enough distance to place text in
            while (distance > target_distance)
            {
               for (double diff = 0; diff < tolerance; diff += tolerance_delta)
               {
                  for(int dir = -1; dir < 2; dir+=2) //-1, +1
                  {
                     //Record details for the start of the string placement
                     int orientation = 0;
                     std::auto_ptr<placement_element> current_placement = get_placement_offset(p, path_positions, path_distances, orientation, index, segment_length - (distance - target_distance) + (diff*dir));

                     //We were unable to place here
                     if (current_placement.get() == NULL)
                        continue;

                     //Apply displacement
                     //NOTE: The text is centered on the line in get_placement_offset, so we are offsetting from there
                     if (displacement != 0)
                     {
                        //Average the angle of all characters and then offset them all by that angle
                        //NOTE: This probably calculates a bad angle due to going around the circle, test this!
                        double anglesum = 0;
                        for (unsigned i = 0; i < current_placement->nodes_.size(); i++)
                        {
                           anglesum += current_placement->nodes_[i].angle;
                        }
                        anglesum /= current_placement->nodes_.size(); //Now it is angle average

                        //Offset all the characters by this angle
                        for (unsigned i = 0; i < current_placement->nodes_.size(); i++)
                        {
                           current_placement->nodes_[i].x += displacement*cos(anglesum+M_PI/2);
                           current_placement->nodes_[i].y += displacement*sin(anglesum+M_PI/2);
                        }
                     }

                     bool status = test_placement(p, current_placement, orientation);

                     if (status) //We have successfully placed one
                     {
                        p.placements.push_back(current_placement.release());
                        update_detector(p);

                        //Totally break out of the loops
                        diff = tolerance;
                        break;
                     }
                     else
                     {
                        //If we've failed to place, remove all the envelopes we've added up
                        while (!p.envelopes.empty())
                           p.envelopes.pop();
                     }

                     //Don't need to loop twice when diff = 0
                     if (diff == 0)
                        break;
                  }
               }

               distance -= target_distance; //Consume the spacing gap we have used up
               target_distance = spacing; //Need to reset the target_distance as it is spacing/2 for the first label.
            }
         }

         old_x = new_x;
         old_y = new_y;
      }
   }

   template <typename DetectorT>
   std::auto_ptr<placement_element> placement_finder<DetectorT>::get_placement_offset(placement & p, const std::vector<vertex2d> &path_positions, const std::vector<double> &path_distances, int &orientation, unsigned index, double distance)
   {
      //Check that the given distance is on the given index and find the correct index and distance if not
      while (distance < 0 && index > 1)
      {
         index--;
         distance += path_distances[index];
      }
      if (index <= 1 && distance < 0) //We've gone off the start, fail out
         return std::auto_ptr<placement_element>(NULL);

      //Same thing, checking if we go off the end
      while (index < path_distances.size() && distance > path_distances[index])
      {
         distance -= path_distances[index];
         index++;
      }
      if (index >= path_distances.size())
         return std::auto_ptr<placement_element>(NULL);

      //Keep track of the initial index,distance incase we need to re-call get_placement_offset
      const unsigned initial_index = index;
      const double initial_distance = distance;

      std::auto_ptr<placement_element> current_placement(new placement_element);

      double string_height = p.info.get_dimensions().second;
      double old_x = path_positions[index-1].x;
      double old_y = path_positions[index-1].y;

      double new_x = path_positions[index].x;
      double new_y = path_positions[index].y;

      double dx = new_x - old_x;
      double dy = new_y - old_y;

      double segment_length = path_distances[index];
      if (segment_length == 0) {
          // Not allowed to place across on 0 length segments or discontinuities
          return std::auto_ptr<placement_element>(NULL);
      }

      current_placement->starting_x = old_x + dx*distance/segment_length;
      current_placement->starting_y = old_y + dy*distance/segment_length;
      double angle = atan2(-dy, dx);

      bool orientation_forced = (orientation != 0); //Wether the orientation was set by the caller
      if (!orientation_forced)
         orientation = (angle > 0.55*M_PI || angle < -0.45*M_PI) ? -1 : 1;

      unsigned upside_down_char_count = 0; //Count of characters that are placed upside down.

      for (unsigned i = 0; i < p.info.num_characters(); ++i)
      {
         character_info ci;
         unsigned c;

         double last_character_angle = angle;

         // grab the next character according to the orientation
         ci = orientation > 0 ? p.info.at(i) : p.info.at(p.info.num_characters() - i - 1);
         c = ci.character;

         //Coordinates this character will start at
        if (segment_length == 0) {
            // Not allowed to place across on 0 length segments or discontinuities
            return std::auto_ptr<placement_element>(NULL);
        }
         double start_x = old_x + dx*distance/segment_length;
         double start_y = old_y + dy*distance/segment_length;
         //Coordinates this character ends at, calculated below
         double end_x = 0;
         double end_y = 0;

         if (segment_length - distance  >= ci.width)
         {
            //if the distance remaining in this segment is enough, we just go further along the segment
            distance += ci.width;

            end_x = old_x + dx*distance/segment_length;
            end_y = old_y + dy*distance/segment_length;
         }
         else
         {
            //If there isn't enough distance left on this segment
            // then we need to search untill we find the line segment that ends further than ci.width away
            do
            {
               old_x = new_x;
               old_y = new_y;
               index++;
               if (index >= path_positions.size()) //Bail out if we run off the end of the shape
               {
                  //std::clog << "FAIL: Out of space" << std::endl;
                  return std::auto_ptr<placement_element>(NULL);
               }
               new_x = path_positions[index].x;
               new_y = path_positions[index].y;
               dx = new_x - old_x;
               dy = new_y - old_y;

               segment_length = path_distances[index];
            }
            while (std::sqrt(std::pow(start_x - new_x, 2) + std::pow(start_y - new_y, 2)) < ci.width); //Distance from start_ to new_

            //Calculate the position to place the end of the character on
            find_line_circle_intersection(
               start_x, start_y, ci.width,
               old_x, old_y, new_x, new_y,
               end_x, end_y); //results are stored in end_x, end_y

            //Need to calculate distance on the new segment
            distance = std::sqrt(std::pow(old_x - end_x, 2) + std::pow(old_y - end_y, 2));
         }

         //Calculate angle from the start of the character to the end based on start_/end_ position
         angle = atan2(start_y-end_y, end_x-start_x);

         //Test last_character_angle vs angle
         // since our rendering angle has changed then check against our
         // max allowable angle change.
         double angle_delta = last_character_angle - angle;
         // normalise between -180 and 180
         while (angle_delta > M_PI)
            angle_delta -= 2*M_PI;
         while (angle_delta < -M_PI)
            angle_delta += 2*M_PI;
         if (p.max_char_angle_delta > 0 &&
               fabs(angle_delta) > p.max_char_angle_delta*(M_PI/180))
         {
            //std::clog << "FAIL: Too Bendy!" << std::endl;
            return std::auto_ptr<placement_element>(NULL);
         }

         double render_angle = angle;

         double render_x = start_x;
         double render_y = start_y;

         //Center the text on the line
         render_x -= (((double)string_height/2.0) - 1.0)*cos(render_angle+M_PI/2);
         render_y += (((double)string_height/2.0) - 1.0)*sin(render_angle+M_PI/2);

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

         //Normalise to 0 <= angle < 2PI
         while (render_angle >= 2*M_PI)
            render_angle -= 2*M_PI;
         while (render_angle < 0)
            render_angle += 2*M_PI;

         if (render_angle > M_PI/2 && render_angle < 1.5*M_PI)
            upside_down_char_count++;
      }

      //If we placed too many characters upside down
      if (upside_down_char_count >= p.info.num_characters()/2.0)
      {
         //if we auto-detected the orientation then retry with the opposite orientation
         if (!orientation_forced)
         {
            orientation = -orientation;
            current_placement = get_placement_offset(p, path_positions, path_distances, orientation, initial_index, initial_distance);
         }
         else
         {
            //Otherwise we have failed to find a placement
            //std::clog << "FAIL: Double upside-down!" << std::endl;
            return std::auto_ptr<placement_element>(NULL);
         }
      }

      return current_placement;
   }

   template <typename DetectorT>
   bool placement_finder<DetectorT>::test_placement(placement & p, const std::auto_ptr<placement_element> & current_placement, const int & orientation)
   {
      std::pair<double, double> string_dimensions = p.info.get_dimensions();

      double string_height = string_dimensions.second;


      //Create and test envelopes
      bool status = true;
      for (unsigned i = 0; i < p.info.num_characters(); ++i)
      {
         // grab the next character according to the orientation
         character_info ci = orientation > 0 ? p.info.at(i) : p.info.at(p.info.num_characters() - i - 1);
         int c;
         double x, y, angle;
         current_placement->vertex(&c, &x, &y, &angle);
         x = current_placement->starting_x + x;
         y = current_placement->starting_y - y;
         if (orientation < 0)
         {
            // rotate in place
            x += ci.width*cos(angle) - (string_height-2)*sin(angle);
            y -= ci.width*sin(angle) + (string_height-2)*cos(angle);
            angle += M_PI;
         }

         Envelope<double> e;
         if (p.has_dimensions)
         {
            e.init(x, y, x + p.dimensions.first, y + p.dimensions.second);
         }
         else
         {
            // put four corners of the letter into envelope
            e.init(x, y, x + ci.width*cos(angle),
                     y - ci.width*sin(angle));
            e.expand_to_include(x - ci.height*sin(angle),
                                 y - ci.height*cos(angle));
            e.expand_to_include(x + (ci.width*cos(angle) - ci.height*sin(angle)),
                                 y - (ci.width*sin(angle) + ci.height*cos(angle)));
         }

         if (!dimensions_.intersects(e) ||
               !detector_.has_placement(e, p.info.get_string(), p.minimum_distance))
         {
            //std::clog << "No Intersects:" << !dimensions_.intersects(e) << ": " << e << " @ " << dimensions_ << std::endl;
            //std::clog << "No Placements:" << !detector_.has_placement(e, p.info.get_string(), p.minimum_distance) << std::endl;
            status = false;
            break;
         }

         if (p.avoid_edges && !dimensions_.contains(e))
         {
            //std::clog << "Fail avoid edges" << std::endl;
            status = false;
            break;
         }

         p.envelopes.push(e);
      }

      current_placement->rewind();

      return status;
   }

   template <typename DetectorT>
   void placement_finder<DetectorT>::find_line_circle_intersection(
      const double &cx, const double &cy, const double &radius,
      const double &x1, const double &y1, const double &x2, const double &y2,
      double &ix, double &iy)
   {
      double dx = x2 - x1;
      double dy = y2 - y1;

      double A = dx * dx + dy * dy;
      double B = 2 * (dx * (x1 - cx) + dy * (y1 - cy));
      double C = (x1 - cx) * (x1 - cx) + (y1 - cy) * (y1 - cy) - radius * radius;

      double det = B * B - 4 * A * C;
      if (A <= 0.0000001 || det < 0)
      {
         //Should never happen
         //' No real solutions.
         return;
      }
      else if (det == 0)
      {
         //Could potentially happen....
         //One solution.
         double t = -B / (2 * A);
         ix = x1 + t * dx;
         iy = y1 + t * dy;
         return;
      }
      else
      {
         //Two solutions.

         //Always use the 1st one
         //We only really have one solution here, as we know the line segment will start in the circle and end outside
         double t = (-B + std::sqrt(det)) / (2 * A);
         ix = x1 + t * dx;
         iy = y1 + t * dy;

         //t = (-B - std::sqrt(det)) / (2 * A);
         //ix = x1 + t * dx;
         //iy = y1 + t * dy;

         return;
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
   void placement_finder<DetectorT>::clear()
   {
      detector_.clear();
   }

   typedef coord_transform2<CoordTransform,geometry2d> PathType;
   typedef label_collision_detector4 DetectorType;

   template class placement_finder<DetectorType>;
   template void placement_finder<DetectorType>::find_point_placements<PathType> (placement&, PathType & );
   template void placement_finder<DetectorType>::find_line_placements<PathType> (placement&, PathType & );

}  // namespace
