/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/config.hpp>
#include <mapnik/util/noncopyable.hpp>

// agg
#include "agg_basics.h"

// stl
#include <vector>
#include <memory>
#include <map>

namespace mapnik
{

class vertex_cache;
using vertex_cache_ptr = std::unique_ptr<vertex_cache>;

// Caches all path points and their lengths. Allows easy moving in both directions.
class MAPNIK_DECL vertex_cache : util::noncopyable
{
    struct segment
    {
        segment(double x, double y, double _length) : pos(x, y), length(_length) {}
        pixel_position pos; //Last point of this segment, first point is implicitly defined by the previous segement in this vector
        double length;
    };

    // The first segment always has the length 0 and just defines the starting point.
    struct segment_vector
    {
        segment_vector() : vector(), length(0.) {}
        void add_segment(double x, double y, double len) {
            if (len == 0. && !vector.empty()) return; //Don't add zero length segments
            vector.emplace_back(x, y, len);
            length += len;
        }
        using iterator = std::vector<segment>::iterator;
        std::vector<segment> vector;
        double length;
    };

public:
    // This class has no public members to avoid acciedential modification.
    // It should only be used with save_state/restore_state.
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

    class scoped_state : util::noncopyable
    {
    public:
        scoped_state(vertex_cache &pp) : pp_(pp), state_(pp.save_state()), restored_(false) {}
        void restore() { pp_.restore_state(state_); restored_ = true; }
        ~scoped_state() { if (!restored_) pp_.restore_state(state_); }
        state const& get_state() const { return state_; }
    private:
        vertex_cache &pp_;
        state state_;
        bool restored_;
    };

    ///////////////////////////////////////////////////////////////////////

    template <typename T> vertex_cache(T &path);
    vertex_cache(vertex_cache && rhs);

    double length() const { return current_subpath_->length; }


    pixel_position const& current_position() const { return current_position_; }
    double angle(double width=0.);
    double current_segment_angle();
    double linear_position() const { return position_; }


    // Returns a parallel line in the specified distance.
    vertex_cache & get_offseted(double offset, double region_width);


    // Skip a certain amount of space.
    // This functions automatically calculate new points if the position is not exactly
    // on a point on the path.

    bool forward(double length);
    // Go backwards.
    bool backward(double length);
    // Move in any direction (based on sign of length). Returns false if it reaches either end of the path.
    bool move(double length);
    // Move to given distance.
    bool move_to_distance(double distance);
    // Work on next subpath. Returns false if the is no next subpath.
    bool next_subpath();

    // Compatibility with standard path interface
    void rewind(unsigned);
    unsigned vertex(double *x, double *y);

    // State
    state save_state() const;
    void restore_state(state const& s);
    // Go back to initial state.
    void reset();

    // position on this line closest to the target position
    double position_closest_to(pixel_position const &target_pos);

private:
    void rewind_subpath();
    bool next_segment();
    bool previous_segment();
    void find_line_circle_intersection(
        double cx, double cy, double radius,
        double x1, double y1, double x2, double y2,
        double & ix, double & iy) const;
    // Position as calculated by last move/forward/next call.
    pixel_position current_position_;
    // First pixel of current segment.
    pixel_position segment_starting_point_;
    // List of all subpaths.
    std::vector<segment_vector> subpaths_;
    // Currently active subpath.
    std::vector<segment_vector>::iterator current_subpath_;
    // Current segment for normal operation (move()).
    segment_vector::iterator current_segment_;
    // Current segment in compatibility mode (vertex(), rewind()).
    segment_vector::iterator vertex_segment_;
    // Currently active subpath in compatibility mode.
    std::vector<segment_vector>::iterator vertex_subpath_;
    // State is initialized (after first call to next_subpath()).
    bool initialized_;
    // Position from start of segment.
    double position_in_segment_;
    // Angle for current segment.
    mutable double angle_;
    // Is the value in angle_ valid?
    // Used to avoid unnecessary calculations.
    mutable bool angle_valid_;
    using offseted_lines_map = std::map<double, vertex_cache_ptr>;
    // Cache of all offseted lines already computed.
    offseted_lines_map offseted_lines_;
    // Linear position, i.e distance from start of line.
    double position_;
};


template <typename T>
vertex_cache::vertex_cache(T & path)
        : current_position_(),
          segment_starting_point_(),
          subpaths_(),
          current_subpath_(),
          current_segment_(),
          vertex_segment_(),
          vertex_subpath_(),
          initialized_(false),
          position_in_segment_(0.),
          angle_(0.),
          angle_valid_(false),
          offseted_lines_(),
          position_(0.)
{
    path.rewind(0);
    unsigned cmd;
    double new_x = 0., new_y = 0., old_x = 0., old_y = 0.;
    bool first = true; //current_subpath_ uninitalized
    while (!agg::is_stop(cmd = path.vertex(&new_x, &new_y)))
    {
        if (agg::is_move_to(cmd))
        {
            //Create new sub path
            subpaths_.emplace_back();
            current_subpath_ = subpaths_.end()-1;
            current_subpath_->add_segment(new_x, new_y, 0);
            first = false;
        }
        else if (agg::is_line_to(cmd))
        {
            if (first)
            {
                MAPNIK_LOG_ERROR(vertex_cache) << "No starting point in path!\n";
                continue;
            }
            double dx = old_x - new_x;
            double dy = old_y - new_y;
            double segment_length = std::sqrt(dx*dx + dy*dy);
            current_subpath_->add_segment(new_x, new_y, segment_length);
        }
        else if (agg::is_closed(cmd) && !current_subpath_->vector.empty())
        {
            segment const & first_segment = current_subpath_->vector[0];
            double dx = old_x - first_segment.pos.x;
            double dy = old_y - first_segment.pos.y;
            double segment_length = std::sqrt(dx*dx + dy*dy);
            current_subpath_->add_segment(first_segment.pos.x, first_segment.pos.y, segment_length);
        }
        old_x = new_x;
        old_y = new_y;
    }
}

}
#endif
