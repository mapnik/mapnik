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
         placement(string_info & info_, shield_symbolizer const& sym, bool has_dimensions_= false);

         placement(string_info & info_, text_symbolizer const& sym);

         ~placement();

         string_info & info;

         position displacement_;
         label_placement_e label_placement;

         std::queue< Envelope<double> > envelopes;

         //output
         boost::ptr_vector<placement_element> placements;

         int wrap_width;
         bool wrap_before; // wraps text at wrap_char immediately before current word
         unsigned char wrap_char;
         int text_ratio;

         int label_spacing; // distance between repeated labels on a single geometry
         unsigned label_position_tolerance; //distance the label can be moved on the line to fit, if 0 the default is used
         bool force_odd_labels; //Always try render an odd amount of labels

         double max_char_angle_delta;
         double minimum_distance;
         bool avoid_edges;
         bool has_dimensions;
         bool allow_overlap;
         std::pair<double, double> dimensions;
         int text_size;
   };



   template <typename DetectorT>
   class placement_finder : boost::noncopyable
   {
   public:
      placement_finder(DetectorT & detector);

      //Try place a single label at the given point
      void find_point_placement(placement & p, double pos_x, double pos_y, vertical_alignment_e = MIDDLE, unsigned line_spacing=0, unsigned character_spacing=0, horizontal_alignment_e = H_MIDDLE, justify_alignment_e = J_MIDDLE);

      //Iterate over the given path, placing point labels with respect to label_spacing
      template <typename T>
      void find_point_placements(placement & p, T & path);

      //Iterate over the given path, placing line-following labels with respect to label_spacing
      template <typename T>
      void find_line_placements(placement & p, T & path);

      void update_detector(placement & p);

      void clear();

   private:
      ///Helpers for find_line_placement

      ///Returns a possible placement on the given line, does not test for collisions
      //index: index of the node the current line ends on
      //distance: distance along the given index that the placement should start at, this includes the offset,
      //          as such it may be > or < the length of the current line, so this must be checked for
      //orientation: if set to != 0 the placement will be attempted with the given orientation
      //             otherwise it will autodetect the orientation.
      //             If >= 50% of the characters end up upside down, it will be retried the other way.
      //             RETURN: 1/-1 depending which way up the string ends up being.
      std::auto_ptr<placement_element> get_placement_offset(placement & p,
                                                            const std::vector<vertex2d> & path_positions,
                                                            const std::vector<double> & path_distances,
                                                            int & orientation, unsigned index, double distance);

      ///Tests wether the given placement_element be placed without a collision
      // Returns true if it can
      // NOTE: This edits p.envelopes so it can be used afterwards (you must clear it otherwise)
      bool test_placement(placement & p, const std::auto_ptr<placement_element> & current_placement, const int & orientation);

      ///Does a line-circle intersect calculation
      // NOTE: Follow the strict pre conditions
      // Pre Conditions: x1,y1 is within radius distance of cx,cy. x2,y2 is outside radius distance of cx,cy
      //                 This means there is exactly one intersect point
      // Result is returned in ix, iy
      void find_line_circle_intersection(
         const double &cx, const double &cy, const double &radius,
         const double &x1, const double &y1, const double &x2, const double &y2,
         double &ix, double &iy);

      ///General Internals



      DetectorT & detector_;
      Envelope<double> const& dimensions_;
   };
}

#endif

