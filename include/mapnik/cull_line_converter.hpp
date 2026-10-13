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

#ifndef MAPNIK_CULL_LINE_CONVERTER_HPP
#define MAPNIK_CULL_LINE_CONVERTER_HPP

#include <mapnik/geometry/box2d.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
MAPNIK_DISABLE_WARNING_POP

// stl
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace mapnik {

// Removes the parts of a path that are far outside a box, ahead of a stroker.
//
// A segment is kept when its bounding box intersects the box, and so are the
// segments around each run of kept ones up to two non-degenerate segments on
// either side. Every vertex near the box therefore keeps the same neighbours
// and gets the same joins, and each outline edge that can reach the box is
// generated exactly as before. The
// box must already include the stroke's reach (the furthest a join or cap
// extends from its vertex) beyond the rasterizer clip area: what is dropped,
// and the caps added where a path is cut, then lie entirely outside that area
// and the rasterized coverage inside it is unchanged.
template<typename Geometry>
class cull_line
{
  public:
    explicit cull_line(Geometry& geom)
        : geom_(geom)
    {}

    void set_box(box2d<double> const& box) { box_ = box; }

    void rewind(unsigned path_id)
    {
        geom_.rewind(path_id);
        out_.clear();
        pos_ = 0;
        have_next_ = false;
        done_ = false;
    }

    unsigned vertex(double* x, double* y)
    {
        while (pos_ == out_.size())
        {
            if (done_)
                return agg::path_cmd_stop;
            read_path();
        }
        vertex_type const& v = out_[pos_++];
        *x = v.x;
        *y = v.y;
        return v.cmd;
    }

  private:
    struct vertex_type
    {
        double x;
        double y;
        unsigned cmd;
    };

    // Written as negated comparisons so that NaN coordinates count as touching.
    bool touches(vertex_type const& a, vertex_type const& b) const
    {
        return !(std::max(a.x, b.x) < box_.minx() || std::min(a.x, b.x) > box_.maxx() ||
                 std::max(a.y, b.y) < box_.miny() || std::min(a.y, b.y) > box_.maxy());
    }

    // Passes over the rest of the subpath starting at first when none of its
    // segments touches the box, testing them as they come from the source
    // without storing them (when the source can, see mapnik's vertex adapters).
    bool skip_far_rest(vertex_type const& first)
    {
        vertex_type prev = first;
        return geom_.skip_rest_if([&](double x, double y) {
            vertex_type const cur{x, y, agg::path_cmd_line_to};
            bool const far = !touches(prev, cur);
            prev = cur;
            return far;
        });
    }

    void read_path()
    {
        out_.clear();
        pos_ = 0;
        path_.clear();
        vertex_type v;
        if (have_next_)
        {
            path_.push_back(next_);
            have_next_ = false;
        }
        else
        {
            v.cmd = geom_.vertex(&v.x, &v.y);
            if (agg::is_stop(v.cmd))
            {
                done_ = true;
                return;
            }
            path_.push_back(v);
        }
        if constexpr (requires { geom_.skip_rest_if([](double, double) { return true; }); })
        {
            if (agg::is_move_to(path_[0].cmd) && skip_far_rest(path_[0]))
            {
                // no segment of the subpath touches the box: culling drops all of it
                path_.clear();
                return;
            }
        }
        for (;;)
        {
            v.cmd = geom_.vertex(&v.x, &v.y);
            if (agg::is_stop(v.cmd))
            {
                done_ = true;
                break;
            }
            if (agg::is_move_to(v.cmd))
            {
                next_ = v;
                have_next_ = true;
                break;
            }
            path_.push_back(v);
        }
        cull_path();
    }

    void cull_path()
    {
        std::size_t n = path_.size();
        bool closed = false;
        if (agg::is_end_poly(path_[n - 1].cmd))
        {
            if (!agg::is_closed(path_[n - 1].cmd))
            {
                out_.swap(path_);
                return;
            }
            closed = true;
            --n;
        }
        if (n < 2 || !agg::is_move_to(path_[0].cmd))
        {
            out_.swap(path_);
            return;
        }
        for (std::size_t i = 1; i < n; ++i)
        {
            if (!agg::is_line_to(path_[i].cmd))
            {
                out_.swap(path_);
                return;
            }
        }

        std::size_t const segs = closed ? n : n - 1;
        keep_.assign(segs, 0);
        bool any = false;
        bool all = true;
        for (std::size_t i = 0; i < segs; ++i)
        {
            bool const k = touches(path_[i], path_[i + 1 == n ? 0 : i + 1]);
            keep_[i] = k;
            any |= k;
            all &= k;
        }
        if (all)
        {
            out_.swap(path_);
            return;
        }
        if (!any)
            return;

        // Widen every kept run on each side until it includes two segments that
        // are not degenerate: the stroker drops coincident vertices, so it is
        // those that give the vertices next to the run their neighbours.
        wide_.assign(segs, 0);
        std::size_t const laps = closed ? 2 * segs : segs;
        for (int dir = 0; dir < 2; ++dir)
        {
            int count = 2; // non-degenerate segments since the last kept one
            for (std::size_t k = 0; k < laps; ++k)
            {
                std::size_t const i = (dir == 0 ? k : laps - 1 - k) % segs;
                if (keep_[i])
                {
                    wide_[i] = 1;
                    count = 0;
                }
                else if (count < 2)
                {
                    wide_[i] = 1;
                    std::size_t const j = i + 1 == n ? 0 : i + 1;
                    double const dx = path_[j].x - path_[i].x;
                    double const dy = path_[j].y - path_[i].y;
                    if (!(dx * dx + dy * dy <= 1e-18))
                        ++count;
                }
            }
        }
        bool all_wide = true;
        for (std::size_t i = 0; i < segs; ++i)
            all_wide &= wide_[i] != 0;
        if (all_wide)
        {
            out_.swap(path_);
            return;
        }

        // emit each run of kept segments as an open path; for a closed path,
        // start just after a dropped segment so that no run wraps around
        std::size_t start = 0;
        if (closed)
        {
            while (wide_[start])
                ++start;
            ++start;
        }
        std::size_t step = 0;
        while (step < segs)
        {
            std::size_t i = (start + step) % segs;
            if (!wide_[i])
            {
                ++step;
                continue;
            }
            out_.push_back({path_[i].x, path_[i].y, agg::path_cmd_move_to});
            while (step < segs && wide_[(start + step) % segs])
            {
                std::size_t const j = ((start + step) % segs + 1) % n;
                out_.push_back({path_[j].x, path_[j].y, agg::path_cmd_line_to});
                ++step;
            }
        }
    }

    Geometry& geom_;
    box2d<double> box_;
    std::vector<vertex_type> path_;
    std::vector<vertex_type> out_;
    std::vector<std::uint8_t> keep_;
    std::vector<std::uint8_t> wide_;
    std::size_t pos_ = 0;
    vertex_type next_{0.0, 0.0, agg::path_cmd_stop};
    bool have_next_ = false;
    bool done_ = false;
};

} // namespace mapnik

#endif // MAPNIK_CULL_LINE_CONVERTER_HPP
