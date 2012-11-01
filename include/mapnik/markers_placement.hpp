/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_MARKERS_PLACEMENT_HPP
#define MAPNIK_MARKERS_PLACEMENT_HPP

// mapnik
#include <mapnik/markers_placement.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/global.hpp> //round
#include <mapnik/box2d.hpp>

// boost
#include <boost/utility.hpp>

// agg
#include "agg_basics.h"
#include "agg_conv_clip_polygon.h"
#include "agg_conv_clip_polyline.h"
#include "agg_trans_affine.h"
#include "agg_conv_transform.h"
#include "agg_conv_smooth_poly1.h"

// stl
#include <cmath>

namespace mapnik {

template <typename Locator, typename Detector>
class markers_placement : boost::noncopyable
{
public:
    /** Constructor for markers_placement object.
     * \param locator  Path along which markers are placed (type: vertex source)
     * \param size     Size of the marker
     * \param tr       Affine transform
     * \param detector Collision detection
     * \param spacing  Distance between markers. If the value is negative it is
     *                 converted to a positive value with similar magnitude, but
     *                 choosen to optimize marker placement. 0 = no markers
     */
    markers_placement(Locator &locator, box2d<double> const& size, agg::trans_affine const& tr, Detector &detector, double spacing, double max_error, bool allow_overlap)
      : locator_(locator),
        size_(size),
        tr_(tr),
        detector_(detector),
        max_error_(max_error),
        allow_overlap_(allow_overlap)
    {
      marker_width_ = (size_ * tr_).width();
      if (spacing >= 0)
      {
          spacing_ = spacing;
      } else if (spacing < 0)
      {
          spacing_ = find_optimal_spacing(-spacing);
      }
      rewind();
    }

    /** Start again at first marker.
     * \note Returns the same list of markers only works when they were NOT added
     *       to the detector.
     */
    void rewind()
    {
        locator_.rewind(0);
        //Get first point
        done_ = agg::is_stop(locator_.vertex(&next_x, &next_y)) || spacing_ < marker_width_;
        last_x = next_x;
        last_y = next_y; // Force request of new segment
        error_ = 0;
        marker_nr_ = 0;
    }

    /** Get a point where the marker should be placed.
     * Each time this function is called a new point is returned.
     * \param x     Return value for x position
     * \param y     Return value for x position
     * \param angle Return value for rotation angle
     * \param add_to_detector Add selected position to detector
     * \return True if a place is found, false if none is found.
     */
    bool get_point(double & x, double  & y, double & angle,  bool add_to_detector = true)
    {
        if (done_)
        {
            return false;
        }
        unsigned cmd;
        /* This functions starts at the position of the previous marker,
           walks along the path, counting how far it has to go in spacing_left.
           If one marker can't be placed at the position it should go to it is
           moved a bit. The error is compensated for in the next call to this
           function.

           error > 0: Marker too near to the end of the path.
           error = 0: Perfect position.
           error < 0: Marker too near to the beginning of the path.
        */
        if (marker_nr_++ == 0)
        {
            //First marker
            spacing_left_ = spacing_ / 2;
        }
        else
        {
            spacing_left_ = spacing_;
        }
        spacing_left_ -= error_;
        error_ = 0;
        //Loop exits when a position is found or when no more segments are available
        while (true)
        {
            //Do not place markers too close to the beginning of a segment
            if (spacing_left_ < marker_width_/2)
            {
                set_spacing_left(marker_width_/2); //Only moves forward
            }
            //Error for this marker is too large. Skip to the next position.
            if (abs(error_) > max_error_ * spacing_)
            {
                if (error_ > spacing_)
                {
                    error_ = 0; //Avoid moving backwards
                    MAPNIK_LOG_WARN(markers_placement) << "Extremely large error in markers_placement. Please file a bug report.";
                }
                spacing_left_ += spacing_ - error_;
                error_ = 0;
            }
            double dx = next_x - last_x;
            double dy = next_y - last_y;
            double segment_length = std::sqrt(dx * dx + dy * dy);
            if (segment_length <= spacing_left_)
            {
                //Segment is too short to place marker. Find next segment
                spacing_left_ -= segment_length;
                last_x = next_x;
                last_y = next_y;
                while (agg::is_move_to(cmd = locator_.vertex(&next_x, &next_y)))
                {
                    //Skip over "move" commands
                    last_x = next_x;
                    last_y = next_y;
                }
                if (agg::is_stop(cmd))
                {
                    done_ = true;
                    return false;
                }
                continue; //Try again
            }
            /* At this point we know the following things:
               - segment_length > spacing_left
               - error is small enough
               - at least half a marker fits into this segment
            */
            //Check if marker really fits in this segment
            if (segment_length < marker_width_)
            {
                //Segment to short => Skip this segment
                set_spacing_left(segment_length + marker_width_/2); //Only moves forward
                continue;
            }
            else if (segment_length - spacing_left_ < marker_width_/2)
            {
                //Segment is long enough, but we are to close to the end
                //Note: This function moves backwards. This could lead to an infinite
                // loop when another function adds a positive offset. Therefore we
                // only move backwards when there is no offset
                if (error_ == 0)
                {
                    set_spacing_left(segment_length - marker_width_/2, true);
                }
                else
                {
                    //Skip this segment
                    set_spacing_left(segment_length + marker_width_/2); //Only moves forward
                }
                continue; //Force checking of max_error constraint
            }
            angle = atan2(dy, dx);
            x = last_x + dx * (spacing_left_ / segment_length);
            y = last_y + dy * (spacing_left_ / segment_length);
            box2d<double> box = perform_transform(angle, x, y);
            if (!allow_overlap_ && !detector_.has_placement(box))
            {
                //10.0 is the approxmiate number of positions tried and choosen arbitrarily
                set_spacing_left(spacing_left_ + spacing_ * max_error_ / 10.0); //Only moves forward
                continue;
            }
            if (add_to_detector) detector_.insert(box);
            last_x = x;
            last_y = y;
            return true;
        }
    }

private:
    Locator &locator_;
    box2d<double> size_;
    agg::trans_affine tr_;
    Detector &detector_;
    double spacing_;
    double marker_width_;
    double max_error_;
    bool allow_overlap_;

    bool done_;
    double last_x, last_y;
    double next_x, next_y;
    /** If a marker could not be placed at the exact point where it should
     * go the next marker's distance will be a bit lower. */
    double error_;
    double spacing_left_;
    unsigned marker_nr_;

    /** Rotates the size_ box and translates the position. */
    box2d<double> perform_transform(double angle, double dx, double dy)
    {
        double x1 = size_.minx();
        double x2 = size_.maxx();
        double y1 = size_.miny();
        double y2 = size_.maxy();
        agg::trans_affine tr = tr_ * agg::trans_affine_rotation(angle).translate(dx, dy);
        double xA = x1, yA = y1, xB = x2, yB = y1, xC = x2, yC = y2, xD = x1, yD = y2;
        tr.transform(&xA, &yA);
        tr.transform(&xB, &yB);
        tr.transform(&xC, &yC);
        tr.transform(&xD, &yD);
        box2d<double> result(xA, yA, xC, yC);
        result.expand_to_include(xB, yB);
        result.expand_to_include(xD, yD);
        return result;
    }

    /** Automatically chooses spacing. */
    double find_optimal_spacing(double s)
    {
        rewind();
        //Calculate total path length
        unsigned cmd = agg::path_cmd_move_to;
        double length = 0;
        while (!agg::is_stop(cmd))
        {
            double dx = next_x - last_x;
            double dy = next_y - last_y;
            length += std::sqrt(dx * dx + dy * dy);
            last_x = next_x;
            last_y = next_y;
            while (agg::is_move_to(cmd = locator_.vertex(&next_x, &next_y)))
            {
                //Skip over "move" commands
                last_x = next_x;
                last_y = next_y;
            }
        }
        unsigned points = round(length / s);
        if (points == 0) return 0.0; //Path to short
        return length / points;
    }

    /** Set spacing_left_, adjusts error_ and performs sanity checks. */
    void set_spacing_left(double sl, bool allow_negative=false)
    {
        double delta_error = sl - spacing_left_;
        if (!allow_negative && delta_error < 0)
        {
            MAPNIK_LOG_WARN(markers_placement) << "Unexpected negative error in markers_placement. Please file a bug report.";
            return;
        }
    #ifdef MAPNIK_DEBUG
        if (delta_error == 0.0)
        {
            MAPNIK_LOG_WARN(markers_placement) << "Not moving at all in set_spacing_left()! Please file a bug report.";
        }
    #endif
        error_ += delta_error;
        spacing_left_ = sl;
    }

};

}

#endif // MAPNIK_MARKERS_PLACEMENT_HPP
