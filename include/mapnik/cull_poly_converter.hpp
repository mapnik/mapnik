/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_CULL_POLY_CONVERTER_HPP
#define MAPNIK_CULL_POLY_CONVERTER_HPP

#include <mapnik/geometry/box2d.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

// Shortens the parts of a polygon outline that lie outside a box, ahead of a
// scanline rasterizer that clips to that box (or to a smaller one).
//
// Of each run of consecutive vertices that are all on the same outer side of
// the box (all left of it, all above it, ...) only the first and the last are
// kept. The rasterizer clips an edge that is entirely left or right of its clip
// box to a vertical edge on the box border and drops an edge that is entirely
// above or below it; the per-row coverage those produce only depends on where
// a run of such edges starts and ends, so the coverage is unchanged.
template<typename Geometry>
class cull_poly
{
  public:
    explicit cull_poly(Geometry& geom)
        : geom_(geom)
    {}

    void set_box(box2d<double> const& box)
    {
        minx_ = box.minx();
        miny_ = box.miny();
        maxx_ = box.maxx();
        maxy_ = box.maxy();
    }

    void rewind(unsigned path_id)
    {
        geom_.rewind(path_id);
        sides_ = 0;
        have_last_ = false;
        have_next_ = false;
    }

    unsigned vertex(double* x, double* y)
    {
        if (have_next_)
        {
            // the vertex that ended a run, after the run's last vertex
            have_next_ = false;
            *x = next_.x;
            *y = next_.y;
            return next_.cmd;
        }
        for (;;)
        {
            unsigned const cmd = geom_.vertex(x, y);
            unsigned const sides = agg::is_vertex(cmd) ? outer_sides(*x, *y) : 0;
            if (agg::is_line_to(cmd) && (sides_ & sides) != 0)
            {
                // still outside on a common side: hold on to the latest vertex only
                sides_ &= sides;
                last_ = {*x, *y, cmd};
                have_last_ = true;
                continue;
            }
            sides_ = agg::is_line_to(cmd) || agg::is_move_to(cmd) ? sides : 0;
            if (have_last_)
            {
                have_last_ = false;
                next_ = {*x, *y, cmd};
                have_next_ = true;
                *x = last_.x;
                *y = last_.y;
                return last_.cmd;
            }
            return cmd;
        }
    }

  private:
    struct vertex_type
    {
        double x;
        double y;
        unsigned cmd;
    };

    enum : unsigned { left = 1, right = 2, above = 4, below = 8 };

    unsigned outer_sides(double x, double y) const
    {
        return (x < minx_ ? left : 0) | (x > maxx_ ? right : 0) | (y < miny_ ? above : 0) | (y > maxy_ ? below : 0);
    }

    Geometry& geom_;
    double minx_ = 0.0;
    double miny_ = 0.0;
    double maxx_ = 0.0;
    double maxy_ = 0.0;
    vertex_type last_{0.0, 0.0, agg::path_cmd_stop};
    vertex_type next_{0.0, 0.0, agg::path_cmd_stop};
    unsigned sides_ = 0;
    bool have_last_ = false;
    bool have_next_ = false;
};

} // namespace mapnik

#endif // MAPNIK_CULL_POLY_CONVERTER_HPP
