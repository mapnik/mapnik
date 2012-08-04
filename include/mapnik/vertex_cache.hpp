/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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
#ifndef MAPNIK_VERTEX_CACHE_HPP
#define MAPNIK_VERTEX_CACHE_HPP

// mapnik
#include <mapnik/pixel_position.hpp>
#include <mapnik/debug.hpp>

// agg
#include "agg_basics.h"

// stl
#include <vector>
#include <utility>

namespace mapnik
{
/** Caches all path points and their lengths. Allows easy moving in both directions. */
class vertex_cache
{
    struct segment
    {
        segment(double x, double y, double length) : pos(x, y), length(length) {}
        pixel_position pos; //Last point of this segment, first point is implicitly defined by the previous segement in this vector
        double length;
    };

    /* The first segment always has the length 0 and just defines the starting point. */
    struct segment_vector
    {
        typedef std::vector<segment>::iterator iterator;
        std::vector<segment> vector;
        double length;
    };
public:
    /** This class has no public members to avoid acciedential modification.
     * It should only be used with save_state/restore_state. */
    class state
    {
        segment_vector::iterator current_segment;
        double position_in_segment;
        pixel_position current_position;
        friend class vertex_cache;
    public:
        pixel_position const& position() { return current_position; }
    };

    template <typename T> vertex_cache(T &path);

    double length() const { return current_subpath_->length; }

    pixel_position const& current_position() const { return current_position_; }

    double angle(double width);

    bool next_subpath();
    bool next_segment();

    /** Moves all positions to a parallel line in the specified distance. */
    void set_offset(double offset);


    /** Skip a certain amount of space.
     *
     * This function automatically calculates new points if the position is not exactly
     * on a point on the path.
     */
    bool forward(double length);

    state save_state() const;
    void restore_state(state s);


private:
    pixel_position current_position_;
    pixel_position segment_starting_point_;
    std::vector<segment_vector> subpaths_;
    std::vector<segment_vector>::iterator current_subpath_;
    segment_vector::iterator current_segment_;
    bool first_subpath_;
    double position_in_segment_;
    mutable double angle_;
    mutable bool angle_valid_;
};

template <typename T>
vertex_cache::vertex_cache(T &path)
        : current_position_(),
          segment_starting_point_(),
          subpaths_(),
          current_subpath_(),
          current_segment_(),
          first_subpath_(true),
          position_in_segment_(0.),
          angle_(0.),
          angle_valid_(false)
{
    path.rewind(0);
    unsigned cmd;
    double new_x = 0., new_y = 0., old_x = 0., old_y = 0.;
    double path_length = 0.;
    bool first = true; //current_subpath_ uninitalized
    while (!agg::is_stop(cmd = path.vertex(&new_x, &new_y)))
    {
        if (agg::is_move_to(cmd))
        {
            if (!first)
            {
                current_subpath_->length = path_length;
            }
            //Create new sub path
            subpaths_.push_back(segment_vector());
            current_subpath_ = subpaths_.end()-1;
            current_subpath_->vector.push_back(segment(new_x, new_y, 0));
            first = false;
        }
        if (agg::is_line_to(cmd))
        {
            if (first)
            {
                MAPNIK_LOG_ERROR(vertex_cache) << "No starting point in path!\n";
                continue;
            }
            double dx = old_x - new_x;
            double dy = old_y - new_y;
            double segment_length = std::sqrt(dx*dx + dy*dy);
            path_length += segment_length;
            current_subpath_->vector.push_back(segment(new_x, new_y, segment_length));
        }
        old_x = new_x;
        old_y = new_y;
    }
    if (!first) {
        current_subpath_->length = path_length;
    } else {
        MAPNIK_LOG_DEBUG(vertex_cache) << "Empty path\n";
    }
}

double vertex_cache::angle(double width)
{
    if (width + position_in_segment_ < current_segment_->length)
    {
        //Only calculate angle on request as it is expensive
        if (!angle_valid_)
        {
            angle_ = atan2(current_segment_->pos.y - segment_starting_point_.y,
                           current_segment_->pos.x - segment_starting_point_.x);
        }
        return angle_;
    } else
    {
        state s = save_state();
        pixel_position const& old_pos = s.position();
        forward(width);
        double angle = atan2(current_position_.y - old_pos.y,
                             current_position_.x - old_pos.x);
        restore_state(s);
        return angle;
    }
}

bool vertex_cache::next_subpath()
{
    if (first_subpath_)
    {
        current_subpath_ = subpaths_.begin();
        first_subpath_ = false;
    } else
    {
        current_subpath_++;
    }
    if (current_subpath_ == subpaths_.end()) return false;
    current_segment_ = current_subpath_->vector.begin();
    //All subpaths contain at least one segment
    current_position_ = current_segment_->pos;
    position_in_segment_ = 0;
    segment_starting_point_ = current_position_;
    angle_valid_ = false;
    return true;
}

bool vertex_cache::next_segment()
{
    segment_starting_point_ = current_segment_->pos; //Next segments starts at the end of the current one
    if (current_segment_ == current_subpath_->vector.end()) return false;
    current_segment_++;
    if (current_segment_ == current_subpath_->vector.end()) return false;
    angle_valid_ = false;
    return true;
}

void vertex_cache::set_offset(double offset)
{
    MAPNIK_LOG_DEBUG(vertex_cache) << "set_offset: unimplemented\n";
}

bool vertex_cache::forward(double length)
{
    if (length < 0)
    {
        MAPNIK_LOG_ERROR(vertex_cache) << "vertex_cache::forward() called with negative argument!\n";
    }
    length += position_in_segment_;
    while (length >= current_segment_->length)
    {
        length -= current_segment_->length;
        if (!next_segment()) return false; //Skip all complete segments
    }
    double factor = length / current_segment_->length;
    position_in_segment_ = length;
    current_position_ = segment_starting_point_ + (current_segment_->pos - segment_starting_point_) * factor;
    return true;
}

vertex_cache::state vertex_cache::save_state() const
{
    state s;
    s.current_segment = current_segment_;
    s.position_in_segment = position_in_segment_;
    s.current_position = current_position_;
    return s;
}

void vertex_cache::restore_state(vertex_cache::state s)
{
    current_segment_ = s.current_segment;
    position_in_segment_ = s.position_in_segment;
    current_position_ = s.current_position;
    angle_valid_ = false;
}


}
#endif
