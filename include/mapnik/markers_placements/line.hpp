/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_MARKERS_PLACEMENTS_LINE_HPP
#define MAPNIK_MARKERS_PLACEMENTS_LINE_HPP

#include <mapnik/markers_placements/point.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/global.hpp> //round

#include <cmath>

namespace mapnik {

template <typename Locator, typename Detector>
class markers_line_placement : public markers_point_placement<Locator, Detector>
{
public:
    markers_line_placement(Locator &locator, Detector &detector, markers_placement_params const& params)
        : markers_point_placement<Locator, Detector>(locator, detector, params),
            last_x(0.0),
            last_y(0.0),
            next_x(0.0),
            next_y(0.0),
            spacing_(0.0),
            marker_width_((params.size * params.tr).width()),
            error_(0.0),
            spacing_left_(0.0),
            marker_nr_(0)
    {
        spacing_ = params.spacing < 1 ? 100 : params.spacing;
        rewind();
    }

    void rewind()
    {
        this->locator_.rewind(0);
        //Get first point
        this->done_ = agg::is_stop(this->locator_.vertex(&next_x, &next_y));
        last_x = next_x;
        last_y = next_y; // Force request of new segment
        error_ = 0.0;
        marker_nr_ = 0;
    }

    bool get_point(double &x, double &y, double &angle, bool ignore_placement)
    {
        if (this->done_)
        {
            return false;
        }

        if (this->locator_.type() == mapnik::geometry_type::types::Point)
        {
            return markers_point_placement<Locator, Detector>::get_point(x, y, angle, ignore_placement);
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
        if (marker_nr_ == 0)
        {
            //First marker
            marker_nr_++;
            spacing_left_ = spacing_ / 2;
        }
        else
        {
            spacing_left_ = spacing_;
        }
        spacing_left_ -= error_;
        error_ = 0.0;
        double max_err_allowed = this->params_.max_error * spacing_;
        //Loop exits when a position is found or when no more segments are available
        while (true)
        {
            //Do not place markers too close to the beginning of a segment
            if (spacing_left_ < this->marker_width_ / 2)
            {
                set_spacing_left(this->marker_width_ / 2); //Only moves forward
            }
            //Error for this marker is too large. Skip to the next position.
            if (std::fabs(error_) > max_err_allowed)
            {
                while (this->error_ > spacing_)
                {
                    error_ -= spacing_; //Avoid moving backwards
                }
                spacing_left_ += spacing_ - this->error_;
                error_ = 0.0;
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
                while (agg::is_move_to(cmd = this->locator_.vertex(&next_x, &next_y)))
                {
                    //Skip over "move" commands
                    last_x = next_x;
                    last_y = next_y;
                }
                if (agg::is_stop(cmd) || cmd == SEG_CLOSE)
                {
                    this->done_ = true;
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
            if (segment_length < this->marker_width_)
            {
                //Segment to short => Skip this segment
                set_spacing_left(segment_length + this->marker_width_/2); //Only moves forward
                continue;
            }
            else if (segment_length - spacing_left_ < this->marker_width_/2)
            {
                //Segment is long enough, but we are to close to the end
                //Note: This function moves backwards. This could lead to an infinite
                // loop when another function adds a positive offset. Therefore we
                // only move backwards when there is no offset
                if (error_ == 0)
                {
                    set_spacing_left(segment_length - this->marker_width_/2, true);
                }
                else
                {
                    //Skip this segment
                    set_spacing_left(segment_length + this->marker_width_/2); //Only moves forward
                }
                continue; //Force checking of max_error constraint
            }
            angle = std::atan2(dy, dx);
            x = last_x + dx * (spacing_left_ / segment_length);
            y = last_y + dy * (spacing_left_ / segment_length);
            box2d<double> box = this->perform_transform(angle, x, y);
            if (this->params_.avoid_edges && !this->detector_.extent().contains(box))
            {
                set_spacing_left(spacing_left_ + spacing_ * this->params_.max_error / 10.0);
                continue;
            }
            if (!this->params_.allow_overlap && !this->detector_.has_placement(box))
            {
                //10.0 is the approxmiate number of positions tried and choosen arbitrarily
                set_spacing_left(spacing_left_ + spacing_ * this->params_.max_error / 10.0); //Only moves forward
                continue;
            }
            if (!ignore_placement)
            {
                this->detector_.insert(box);
            }
            last_x = x;
            last_y = y;
            return true;
        }
    }

private:
    double last_x;
    double last_y;
    double next_x;
    double next_y;
    double spacing_;
    double marker_width_;
    // If a marker could not be placed at the exact point where it should
    // go the next marker's distance will be a bit lower.
    double error_;
    double spacing_left_;
    unsigned marker_nr_;

    // Set spacing_left_, adjusts error_ and performs sanity checks.
    void set_spacing_left(double sl, bool allow_negative=false)
    {
        double delta_error = sl - spacing_left_;
        if (!allow_negative && delta_error < 0)
        {
            MAPNIK_LOG_WARN(markers_line_placement)
                << "Unexpected negative error in markers_line_placement. "
                   "Please file a bug report.";
            return;
        }
#ifdef MAPNIK_DEBUG
        if (delta_error == 0.0)
        {
            MAPNIK_LOG_WARN(markers_line_placement)
                << "Not moving at all in set_spacing_left()! "
                   "Please file a bug report.";
        }
#endif
        error_ += delta_error;
        spacing_left_ = sl;
    }
};

}

#endif // MAPNIK_MARKERS_PLACEMENTS_LINE_HPP
