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
//#include <utility>
//#include <cmath>

//boost
#include <boost/shared_ptr.hpp>

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
        state const& get_state() const { return state_; }
    private:
        vertex_cache &pp_;
        class state state_;
        bool restored_;
    };

    /********************************************************************************************/

    template <typename T> vertex_cache(T &path);

    double length() const { return current_subpath_->length; }


    pixel_position const& current_position() const { return current_position_; }
    double angle(double width=0.);


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
    bool next_subpath();

    // Compatibility with standard path interface
    void rewind(unsigned);
    unsigned vertex(double *x, double *y);

    //State
    state save_state() const;
    void restore_state(state const& s);


private:
    void rewind_subpath();
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




}
#endif
