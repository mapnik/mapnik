/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Hermann Kraus
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

#ifndef MARKERS_PLACEMENT_HPP
#define MARKERS_PLACEMENT_HPP

#include "agg_basics.h"
#include <mapnik/box2d.hpp>

#include <boost/utility.hpp>
#include <cmath>

namespace mapnik {

template <typename Locator, typename Detector>
class markers_placement : boost::noncopyable
{
public:
    markers_placement(Locator &locator, box2d<double> size, Detector &detector, double spacing, double max_error, bool allow_overlap);
    void rewind();
    bool get_point(double *x, double *y, double *angle, bool add_to_detector = true);
private:
    Locator &locator_;
    box2d<double> size_;
    Detector &detector_;
    double spacing_;
    bool done_;
    double last_x, last_y;
    double next_x, next_y;
    /** If a marker could not be placed at the exact point where it should
     * go the next markers distance will be a bit lower. */
    double error_;
    double max_error_;
    unsigned marker_nr_;
    bool allow_overlap_;
    box2d<double> perform_transform(double angle, double dx, double dy);
    double find_optimal_spacing(double s);
};

/** Constructor for markers_placement object.
 * \param locator  Path along which markers are placed (type: vertex source)
 * \param size     Size of the marker
 * \param detector Collision detection
 * \param spacing  Distance between markers. If the value is negative it is
 *                 converted to a positive value with similar magnitude, but
 *                 choosen to optimize marker placement. 0 = no markers
 */
template <typename Locator,  typename Detector> markers_placement<Locator, Detector>::markers_placement(
    Locator &locator, box2d<double> size, Detector &detector, double spacing, double max_error, bool allow_overlap)
    : locator_(locator), size_(size), detector_(detector), max_error_(max_error), allow_overlap_(allow_overlap)
{
    if (spacing >= 0)
    {
        spacing_ = spacing;
    } else if (spacing < 0)
    {
        spacing_ = find_optimal_spacing(-spacing);
    }
    rewind();
}

/** Autmatically chooses spacing. */
template <typename Locator, typename Detector> double markers_placement<Locator, Detector>::find_optimal_spacing(double s)
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
    if (points == 0) return 0; //Path to short
    return length / points;
}

/** Start again at first marker.
 * \note Returning the same list of markers only works when they were NOT added
 *       to the detector.
 */
template <typename Locator, typename Detector> void markers_placement<Locator, Detector>::rewind()
{
    locator_.rewind(0);
    //Get first point
    done_ = agg::is_stop(locator_.vertex(&next_x, &next_y)) || spacing_ < size_.width();
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
template <typename Locator, typename Detector> bool markers_placement<Locator, Detector>::get_point(
    double *x, double *y, double *angle, bool add_to_detector)
{
    if (done_) return false;
    
    unsigned cmd;
    double spacing_left;
    if (marker_nr_++ == 0)
    {
        spacing_left = spacing_ / 2;
    } else
    {
        spacing_left = spacing_;
    }

    spacing_left -= error_;
    error_ = 0;

    while (true)
    {
        //Loop exits when a position is found or when no more segments are available
        if (spacing_left < size_.width()/2)
        {
            //Do not place markers to close to the beginning of a segment
            error_ += size_.width()/2 - spacing_left;
            spacing_left = size_.width()/2;
        }
        if (abs(error_) > max_error_ * spacing_)
        {
            spacing_left += spacing_ - error_;
            error_ = 0;
        }
        double dx = next_x - last_x;
        double dy = next_y - last_y;
        double d = std::sqrt(dx * dx + dy * dy);
        if (d <= spacing_left)
        {
            //Segment is to short to place marker. Find next segment
            spacing_left -= d;
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
        //Check if marker really fits in this segment
        if (d < size_.width())
        {
            //Segment to short => Skip this segment
            error_ += d + size_.width()/2 - spacing_left;
            spacing_left = d + size_.width()/2;
            continue;
        } else if (d - spacing_left < size_.width()/2)
        {
            //Segment is long enough, but we are to close to the end
            
            //Note: This function moves backwards. This could lead to an infinite
            // loop when another function adds a positive offset. Therefore we
            // only move backwards when there is no offset
            if (error_ == 0)
            {
                error_ += d - size_.width()/2 - spacing_left;
                spacing_left = d - size_.width()/2;
            } else
            {
                //Skip this segment
                error_ += d + size_.width()/2 - spacing_left;
                spacing_left = d + size_.width()/2;
            }
            continue; //Force checking of max_error constraint
        }
        *angle = atan2(dy, dx);
        *x = last_x + dx * (spacing_left / d);
        *y = last_y + dy * (spacing_left / d);

        box2d<double> box = perform_transform(*angle, *x, *y);
        if (!allow_overlap_ && !detector_.has_placement(box))
        {
            //10.0 is choosen arbitrarily
            error_ += spacing_ * max_error_ / 10.0;
            spacing_left += spacing_ * max_error_ / 10.0;
            continue;
        }
        if (add_to_detector) detector_.insert(box);
        last_x = *x;
        last_y = *y;
        return true;
    }
}

/** Rotates the size_ box and translates the position. */
template <typename Locator, typename Detector> box2d<double> markers_placement<Locator, Detector>::perform_transform(double angle, double dx, double dy)
{
    double c = cos(angle), s = sin(angle);
    double x1 = size_.minx();
    double x2 = size_.maxx();
    double y1 = size_.miny();
    double y2 = size_.maxy();

    double x1_ = dx + x1 * c - y1 * s;
    double y1_ = dy + x1 * s + y1 * c;
    double x2_ = dx + x2 * c - y2 * s;
    double y2_ = dy + x2 * s + y2 * c;

    return box2d<double>(x1_, y1_, x2_, y2_);
}
} /* end namespace */
#endif // MARKERS_PLACEMENT_HPP
