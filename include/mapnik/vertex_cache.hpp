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
#include <mapnik/offset_converter.hpp>

// agg
#include "agg_basics.h"

// stl
#include <vector>
#include <utility>
#include <cmath>

//boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace mapnik
{

class vertex_cache;
typedef boost::shared_ptr<vertex_cache> vertex_cache_ptr;

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
        pixel_position segment_starting_point;
        double position_;
        friend class vertex_cache;
    public:
        pixel_position const& position() const { return current_position; }
    };

    class scoped_state
    {
    public:
        scoped_state(vertex_cache &pp) : pp_(pp), state_(pp.save_state()), restored_(false) {}
        void restore() { pp_.restore_state(state_); restored_ = true; }
        ~scoped_state() { if (!restored_) pp_.restore_state(state_); }
        state const& state() const { return state_; }
    private:
        vertex_cache &pp_;
        class state state_;
        bool restored_;
    };

    template <typename T> vertex_cache(T &path);

    double length() const { return current_subpath_->length; }

    pixel_position const& current_position() const { return current_position_; }

    double angle(double width=0.);

    bool next_subpath();

    /** Moves all positions to a parallel line in the specified distance. */
    vertex_cache &get_offseted(double offset, double region_width);


    /** Skip a certain amount of space.
     *
     * This function automatically calculates new points if the position is not exactly
     * on a point on the path.
     */
    bool forward(double length);
    bool backward(double length);
    bool move(double length); //Move works in both directions
    void rewind_subpath();
    // Compatibility with standard path interface
    void rewind(unsigned);
    unsigned vertex(double *x, double *y);

    state save_state() const;
    void restore_state(state const& s);


private:
    bool next_segment();
    bool previous_segment();
    pixel_position current_position_;
    pixel_position segment_starting_point_;
    std::vector<segment_vector> subpaths_;
    std::vector<segment_vector>::iterator current_subpath_;
    segment_vector::iterator current_segment_;
    segment_vector::iterator vertex_segment_; //Only for vertex()
    std::vector<segment_vector>::iterator vertex_subpath_;
    bool first_subpath_;
    double position_in_segment_;
    mutable double angle_;
    mutable bool angle_valid_;
    vertex_cache_ptr offseted_line_;
    double position_;
};


template <typename T>
vertex_cache::vertex_cache(T &path)
        : current_position_(),
          segment_starting_point_(),
          subpaths_(),
          current_subpath_(),
          current_segment_(),
          vertex_segment_(),
          vertex_subpath_(),
          first_subpath_(true),
          position_in_segment_(0.),
          angle_(0.),
          angle_valid_(false),
          offseted_line_(),
          position_(0.)
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
 /* IMPORTANT NOTE: See note about coordinate systems in placement_finder::find_point_placement()
  * for imformation about why the y axis is inverted! */
    double tmp = width + position_in_segment_;
    if ((tmp <= current_segment_->length) && (tmp >= 0))
    {
        //Only calculate angle on request as it is expensive
        if (!angle_valid_)
        {
            angle_ = atan2(-(current_segment_->pos.y - segment_starting_point_.y),
                           current_segment_->pos.x - segment_starting_point_.x);
        }
        return width >= 0 ? angle_ : angle_ + M_PI;
    } else
    {
        scoped_state s(*this);
        pixel_position const& old_pos = s.state().position();
        move(width);
        double angle = atan2(-(current_position_.y - old_pos.y),
                             current_position_.x - old_pos.x);
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
    rewind_subpath();
    return true;
}

bool vertex_cache::next_segment()
{
    segment_starting_point_ = current_segment_->pos; //Next segments starts at the end of the current one
    if (current_segment_ == current_subpath_->vector.end()) return false;
    current_segment_++;
    angle_valid_ = false;
    if (current_segment_ == current_subpath_->vector.end()) return false;
    return true;
}

bool vertex_cache::previous_segment()
{
    if (current_segment_ == current_subpath_->vector.begin()) return false;
    current_segment_--;
    angle_valid_ = false;
    if (current_segment_ == current_subpath_->vector.begin())
    {
        //First segment is special
        segment_starting_point_ = current_segment_->pos;
        return true;
    }
    segment_starting_point_ = (current_segment_-1)->pos;
    return true;
}

vertex_cache & vertex_cache::get_offseted(double offset, double region_width)
{
    if (fabs(offset) < 0.01)
    {
        return *this;
    }
    //TODO: Cache offseted lines
    offset_converter<vertex_cache> converter(*this);
    converter.set_offset(offset);
    offseted_line_ = vertex_cache_ptr(new vertex_cache(converter));
    offseted_line_->rewind_subpath(); //TODO: Multiple subpath support
    double seek = (position_ + region_width/2.) * offseted_line_->length() / length() - region_width/2.;
    if (seek < 0) seek = 0;
    if (seek > offseted_line_->length()) seek = offseted_line_->length();
    offseted_line_->move(seek);
    return *offseted_line_;
}

bool vertex_cache::forward(double length)
{
    if (length < 0)
    {
        MAPNIK_LOG_ERROR(vertex_cache) << "vertex_cache::forward() called with negative argument!\n";
        return false;
    }
    return move(length);
}

bool vertex_cache::backward(double length)
{
    if (length < 0)
    {
        MAPNIK_LOG_ERROR(vertex_cache) << "vertex_cache::backward() called with negative argument!\n";
        return false;
    }
    return move(-length);
}

bool vertex_cache::move(double length)
{
    position_ += length;
    length += position_in_segment_;
    while (length >= current_segment_->length)
    {
        length -= current_segment_->length;
        if (!next_segment()) return false; //Skip all complete segments
    }
    while (length < 0)
    {
        if (!previous_segment()) return false;
        length += current_segment_->length;
    }
    double factor = length / current_segment_->length;
    position_in_segment_ = length;
    current_position_ = segment_starting_point_ + (current_segment_->pos - segment_starting_point_) * factor;
    return true;
}

void vertex_cache::rewind_subpath()
{
    current_segment_ = current_subpath_->vector.begin();
    //All subpaths contain at least one segment
    current_position_ = current_segment_->pos;
    position_in_segment_ = 0;
    segment_starting_point_ = current_position_;
    angle_valid_ = false;
    position_ = 0;
}

void vertex_cache::rewind(unsigned)
{
    vertex_subpath_ = subpaths_.begin();
    vertex_segment_ = vertex_subpath_->vector.begin();
}

unsigned vertex_cache::vertex(double *x, double *y)
{
    if (vertex_segment_ == vertex_subpath_->vector.end())
    {
        vertex_subpath_++;
        if (vertex_subpath_ == subpaths_.end()) return agg::path_cmd_stop;
        vertex_segment_ = vertex_subpath_->vector.begin();
    }
    *x = vertex_segment_->pos.x;
    *y = vertex_segment_->pos.y;
    unsigned cmd = (vertex_segment_ == vertex_subpath_->vector.begin()) ? agg::path_cmd_move_to : agg::path_cmd_line_to;
    vertex_segment_++;
    return cmd;
}


vertex_cache::state vertex_cache::save_state() const
{
    state s;
    s.current_segment = current_segment_;
    s.position_in_segment = position_in_segment_;
    s.current_position = current_position_;
    s.segment_starting_point = segment_starting_point_;
    s.position_ = position_;
    return s;
}

void vertex_cache::restore_state(state const& s)
{
    current_segment_ = s.current_segment;
    position_in_segment_ = s.position_in_segment;
    current_position_ = s.current_position;
    segment_starting_point_ = s.segment_starting_point;
    position_ = s.position_;
    angle_valid_ = false;
}


}
#endif
