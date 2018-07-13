/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_OFFSET_CONVERTER_HPP
#define MAPNIK_OFFSET_CONVERTER_HPP

#ifdef MAPNIK_LOG
#include <mapnik/debug.hpp>
#endif
#include <mapnik/global.hpp>
#include <mapnik/config.hpp>
#include <mapnik/util/math.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/vertex_cache.hpp>

// stl
#include <cmath>
#include <vector>
#include <cstddef>
#include <algorithm>

namespace mapnik
{

static constexpr double offset_converter_default_threshold = 5.0;

template <typename Geometry>
struct offset_converter
{
    using size_type = std::size_t;

    offset_converter(Geometry & geom)
        : geom_(geom)
        , offset_(0.0)
        , threshold_(offset_converter_default_threshold)
        , half_turn_segments_(16)
        , status_(initial)
        , pre_first_(vertex2d::no_init)
        , pre_(vertex2d::no_init)
        , cur_(vertex2d::no_init)
    {}

    enum status
    {
        initial,
        process
    };

    unsigned type() const
    {
        return static_cast<unsigned>(geom_.type());
    }

    double get_offset() const
    {
        return offset_;
    }

    double get_threshold() const
    {
        return threshold_;
    }

    void set_offset(double val)
    {
        if (offset_ != val)
        {
            offset_ = val;
            reset();
        }
    }

    void set_threshold(double val)
    {
        threshold_ = val;
        // no need to reset(), since threshold doesn't affect
        // offset vertices' computation, it only controls how
        // far will we be looking for self-intersections
    }

    unsigned vertex(double * x, double * y)
    {
        if (offset_ == 0.0)
        {
            return geom_.vertex(x, y);
        }

        if (status_ == initial)
        {
            init_vertices();
        }

        if (pos_ >= vertices_.size())
        {
            return SEG_END;
        }

        pre_ = (pos_ ? cur_ : pre_first_);
        cur_ = vertices_.at(pos_++);

        if (pos_ == vertices_.size())
        {
            return output_vertex(x, y);
        }

        double const check_dist = offset_ * threshold_;
        double const check_dist2 = check_dist * check_dist;
        double t = 1.0;
        double vt, ut;

        for (size_t i = pos_; i+1 < vertices_.size(); ++i)
        {
            //break; // uncomment this to see all the curls

            vertex2d const& u0 = vertices_[i];

            // End or beginning of a line or ring must not be filtered out
            // to not to join lines or rings together.
            if (u0.cmd == SEG_CLOSE || u0.cmd == SEG_MOVETO)
            {
                break;
            }

            vertex2d const& u1 = vertices_[i+1];
            double const dx = u0.x - cur_.x;
            double const dy = u0.y - cur_.y;

            if (dx*dx + dy*dy > check_dist2)
            {
                break;
            }

            if (!intersection(pre_, cur_, &vt, u0, u1, &ut))
            {
                continue;
            }

            if (vt < 0.0 || vt > t || ut < 0.0 || ut > 1.0)
            {
                continue;
            }

            t = vt;
            pos_ = i+1;
        }

        cur_.x = pre_.x + t * (cur_.x - pre_.x);
        cur_.y = pre_.y + t * (cur_.y - pre_.y);
        return output_vertex(x, y);
    }

    void reset()
    {
        geom_.rewind(0);
        vertices_.clear();
        status_ = initial;
        pos_ = 0;
    }

    void rewind(unsigned)
    {
        pos_ = 0;
    }

private:

    static double explement_reflex_angle(double angle)
    {
        if (angle > M_PI)
        {
            return angle - 2 * M_PI;
        }
        else if (angle < -M_PI)
        {
            return angle + 2 * M_PI;
        }
        else
        {
            return angle;
        }
    }

    static bool intersection(vertex2d const& u1, vertex2d const& u2, double* ut,
                             vertex2d const& v1, vertex2d const& v2, double* vt)
    {
        double const dx = v1.x - u1.x;
        double const dy = v1.y - u1.y;
        double const ux = u2.x - u1.x;
        double const uy = u2.y - u1.y;
        double const vx = v2.x - v1.x;
        double const vy = v2.y - v1.y;

        // the first line is not vertical
        if (ux < -1e-6 || ux > 1e-6)
        {
            double const up = ux * dy - dx * uy;
            double const dn = vx * uy - ux * vy;

            if (dn > -1e-6 && dn < 1e-6)
            {
                return false; // they are parallel
            }

            *vt = up / dn;
            *ut = (*vt * vx + dx) / ux;
            return true;
        }

        // the first line is not horizontal
        if (uy < -1e-6 || uy > 1e-6)
        {
            double const up = uy * dx - dy * ux;
            double const dn = vy * ux - uy * vx;

            if (dn > -1e-6 && dn < 1e-6)
            {
                return false; // they are parallel
            }

            *vt = up / dn;
            *ut = (*vt * vy + dy) / uy;
            return true;
        }

        // the first line is too short
        return false;
    }

    double joint_angle(double x1x0, double y1y0, double x1x2, double y1y2) const
    {
        double dot = x1x0 * x1x2 + y1y0 * y1y2; // dot product
        double det = x1x0 * y1y2 - y1y0 * x1x2; // determinant
        double angle = std::atan2(det, dot); // atan2(y, x) or atan2(sin, cos)
        // angle in [-tau/2; tau/2]

        if (offset_ > 0.0)
        {
            angle = util::tau - angle; // angle in [tau/2; tau*3/2]
        }
        else if (angle < 0)
        {
            angle += util::tau; // angle in [tau/2; tau]
            // angle may now be equal to tau, because if the original angle
            // is very small, the addition cancels it (epsilon + tau == tau)
        }
        if (angle >= util::tau)
        {
            angle -= util::tau;
        }
        return angle;
    }

    /**
     *  @brief  Translate (vx, vy) by rotated (dx, dy).
     */
    static void displace(vertex2d & v, double dx, double dy, double a)
    {
        v.x += dx * std::cos(a) - dy * std::sin(a);
        v.y += dx * std::sin(a) + dy * std::cos(a);
    }

    /**
     *  @brief  Translate (vx, vy) by rotated (0, -offset).
     */
    void displace(vertex2d & v, double a) const
    {
        v.x -= offset_ * std::sin(a);
        v.y += offset_ * std::cos(a);
    }

    /**
     *  @brief  (vx, vy) := (ux, uy) + rotated (0, -offset)
     */
    void displace(vertex2d & v, vertex2d const& u, double a) const
    {
        v.x = u.x - offset_ * std::sin(a);
        v.y = u.y + offset_ * std::cos(a);
        v.cmd = u.cmd;
    }

    int point_line_position(vertex2d const& a, vertex2d const& b, vertex2d const& point) const
    {
        double position = (b.x - a.x) * (point.y - a.y) - (b.y - a.y) * (point.x - a.x);
        if (position > 1e-6) return 1;
        if (position < -1e-6) return -1;
        return 0;
    }

    void displace2(vertex2d & v1, vertex2d const& v0, vertex2d const& v2, double a, double b) const
    {
        double sa = offset_ * std::sin(a);
        double ca = offset_ * std::cos(a);
        double h = std::tan(0.5 * (b - a));
        double hsa = h * sa;
        double hca = h * ca;
        double abs_offset = std::abs(offset_);
        double hsaca = ca-hsa;
        double hcasa = -sa-hca;
        double abs_hsaca = std::abs(hsaca);
        double abs_hcasa = std::abs(hcasa);
        double abs_hsa = std::abs(hsa);
        double abs_hca = std::abs(hca);

        vertex2d v_tmp(vertex2d::no_init);
        v_tmp.x = v1.x - sa - hca;
        v_tmp.y = v1.y + ca - hsa;
        v_tmp.cmd = v1.cmd;

        int same = point_line_position(v0, v2, v_tmp)*point_line_position(v0, v2, v1);

        if (same >= 0 && std::abs(h) < 10)
        {
            v1.x = v_tmp.x;
            v1.y = v_tmp.y;
        }
        else if ((v0.x-v1.x)*(v0.x-v1.x) + (v0.y-v1.y)*(v0.y-v1.y) +
                (v0.x-v2.x)*(v0.x-v2.x) + (v0.y-v2.y)*(v0.y-v2.y) > offset_*offset_)
        {
            if (abs_hsa > abs_offset || abs_hca > abs_offset)
            {
                double scale = std::max(abs_hsa,abs_hca);
                scale = scale < 1e-6 ? 1. : abs_offset / scale;
                // interpolate hsa, hca to <0,abs_offset>
                hsa = hsa * scale;
                sa = sa * scale;
                hca = hca * scale;
                ca = ca * scale;
            }
            v1.x = v1.x - sa - hca;
            v1.y = v1.y + ca - hsa;
        }
        else
        {
            if (abs_hsaca*abs_hsaca + abs_hcasa*abs_hcasa > abs_offset*abs_offset)
            {
                double d = (abs_hsaca*abs_hsaca + abs_hcasa*abs_hcasa);
                d = d < 1e-6 ? 1. : d;
                double scale = (abs_offset*abs_offset)/d;
                v1.x = v1.x + hcasa*scale;
                v1.y = v1.y + hsaca*scale;
            }
            else
            {
                v1.x = v1.x + hcasa;
                v1.y = v1.y + hsaca;
            }
        }
    }

    status init_vertices()
    {
        if (status_ != initial) // already initialized
        {
            return status_;
        }
        vertex2d v0(vertex2d::no_init);
        vertex2d v1(vertex2d::no_init);
        vertex2d v2(vertex2d::no_init);
        vertex2d w(vertex2d::no_init);
        vertex2d start(vertex2d::no_init);
        vertex2d start_v2(vertex2d::no_init);
        std::vector<vertex2d> points;
        std::vector<vertex2d> close_points;
        bool is_polygon = false;
        std::size_t cpt = 0;
        v0.cmd = geom_.vertex(&v0.x, &v0.y);
        v1 = v0;
        // PUSH INITIAL
        points.push_back(v0);
        if (v0.cmd == SEG_END) // not enough vertices in source
        {
            return status_ = process;
        }
        start = v0;
        while ((v0.cmd = geom_.vertex(&v0.x, &v0.y)) != SEG_END)
        {
            if (v0.cmd == SEG_CLOSE)
            {
                is_polygon = true;
                auto & prev = points.back();
                if (prev.x == start.x && prev.y == start.y)
                {
                    prev.x = v0.x; // hack
                    prev.y = v0.y;
                    prev.cmd = SEG_CLOSE; // account for dupes (line_to(move_to) + close_path) in agg poly clipper
                    std::size_t size = points.size();
                    if (size > 1) close_points.push_back(points[size - 2]);
                    else close_points.push_back(prev);
                    continue;
                }
                else
                {
                    close_points.push_back(v1);
                }
            }
            else if (v0.cmd == SEG_MOVETO)
            {
                start = v0;
            }
            v1 = v0;
            points.push_back(v0);
        }
        // Push SEG_END
        points.push_back(vertex2d(v0.x,v0.y,SEG_END));
        std::size_t i = 0;
        v1 = points[i++];
        v2 = points[i++];
        v0.cmd = v1.cmd;
        v0.x = v1.x;
        v0.y = v1.y;

        if (v2.cmd == SEG_END) // not enough vertices in source
        {
            return status_ = process;
        }

        double angle_a = 0;
        // The vector parts from v1 to v0.
        double v_x1x0 = 0;
        double v_y1y0 = 0;
        // The vector parts from v1 to v2;
        double v_x1x2 = v2.x - v1.x;
        double v_y1y2 = v2.y - v1.y;

        if (is_polygon)
        {
            v_x1x0 = close_points[cpt].x - v1.x;
            v_y1y0 = close_points[cpt].y - v1.y;
            cpt++;
            angle_a = std::atan2(-v_y1y0, -v_x1x0);
        }
        double angle_b = std::atan2(v_y1y2, v_x1x2);
        // Angle between the two vectors
        double curve_angle;

        if (!is_polygon)
        {
            // first vertex
            displace(v1, angle_b);
            push_vertex(v1);
        }
        else
        {
            double joint_angle = this->joint_angle(v_x1x0, v_y1y0, v_x1x2, v_y1y2);
            int bulge_steps = 0;

            if (std::abs(joint_angle) > M_PI)
            {
                curve_angle = explement_reflex_angle(angle_b - angle_a);
                // Bulge steps should be determined by the inverse of the joint angle.
                double half_turns = half_turn_segments_ * std::fabs(curve_angle);
                bulge_steps = 1 + static_cast<int>(std::floor(half_turns / M_PI));
            }

            if (bulge_steps == 0)
            {
                displace2(v1, v0, v2, angle_a, angle_b);
                push_vertex(v1);
            }
            else
            {
                displace(v1, angle_b);
                push_vertex(v1);
            }
        }

        // Sometimes when the first segment is too short, it causes ugly
        // curls at the beginning of the line. To avoid this, we make up
        // a fake vertex two offset-lengths before the first, and expect
        // intersection detection smoothes it out.
        if (!is_polygon)
        {
            pre_first_ = v1;
            displace(pre_first_, -2 * std::fabs(offset_), 0, angle_b);
            start_ = pre_first_;
        }
        else
        {
            pre_first_ = v0;
            start_ = pre_first_;
        }
        start_v2.x = v2.x;
        start_v2.y = v2.y;

        vertex2d tmp_prev(vertex2d::no_init);

        while (i < points.size())
        {
            v1 = v2;
            v2 = points[i++];
            if (v1.cmd == SEG_MOVETO)
            {
                if (is_polygon)
                {
                    v1.x = start_.x;
                    v1.y = start_.y;
                    if (cpt < close_points.size())
                    {
                        v_x1x2 = v1.x - close_points[cpt].x;
                        v_y1y2 = v1.y - close_points[cpt].y;
                        cpt++;
                    }
                    start_v2.x = v2.x;
                    start_v2.y = v2.y;
                }
            }
            if (is_polygon && v2.cmd == SEG_MOVETO)
            {
                start_.x = v2.x;
                start_.y = v2.y;
                v2.x = start_v2.x;
                v2.y = start_v2.y;
            }
            else if (v2.cmd == SEG_END)
            {
                if (!is_polygon) break;
                v2.x = start_v2.x;
                v2.y = start_v2.y;
            }
            else if (v2.cmd == SEG_CLOSE)
            {
                v2.x = start_.x;
                v2.y = start_.y;
            }

            // Switch the previous vector's direction as the origin has changed
            v_x1x0 = -v_x1x2;
            v_y1y0 = -v_y1y2;
            // Calculate new angle_a
            angle_a = std::atan2(v_y1y2, v_x1x2);

            // Calculate the new vector
            v_x1x2 = v2.x - v1.x;
            v_y1y2 = v2.y - v1.y;
            // Calculate the new angle_b
            angle_b = std::atan2(v_y1y2, v_x1x2);

            double joint_angle = this->joint_angle(v_x1x0, v_y1y0, v_x1x2, v_y1y2);
            int bulge_steps = 0;

            if (std::abs(joint_angle) > M_PI)
            {
                curve_angle = explement_reflex_angle(angle_b - angle_a);
                // Bulge steps should be determined by the inverse of the joint angle.
                double half_turns = half_turn_segments_ * std::fabs(curve_angle);
                bulge_steps = 1 + static_cast<int>(std::floor(half_turns / M_PI));
            }

            #ifdef MAPNIK_LOG
            if (bulge_steps == 0)
            {
                // inside turn (sharp/obtuse angle)
                MAPNIK_LOG_DEBUG(ctrans) << "offset_converter:"
                    << " Sharp joint [<< inside turn "
                    << static_cast<int>(util::degrees(joint_angle))
                    << " degrees >>]";
            }
            else
            {
                // outside turn (reflex angle)
                MAPNIK_LOG_DEBUG(ctrans) << "offset_converter:"
                    << " Bulge joint >)) outside turn "
                    << static_cast<int>(util::degrees(joint_angle))
                    << " degrees ((< with " << bulge_steps << " segments";
            }
            #endif
            tmp_prev.cmd = v1.cmd;
            tmp_prev.x = v1.x;
            tmp_prev.y = v1.y;

            if (v1.cmd == SEG_MOVETO)
            {
                if (bulge_steps == 0)
                {
                    displace2(v1, v0, v2, angle_a, angle_b);
                    push_vertex(v1);
                }
                else
                {
                    displace(v1, angle_b);
                    push_vertex(v1);
                }
            }
            else
            {
                if (bulge_steps == 0)
                {
                    displace2(v1, v0, v2, angle_a, angle_b);
                    push_vertex(v1);
                }
                else
                {
                    displace(w, v1, angle_a);
                    w.cmd = SEG_LINETO;
                    push_vertex(w);
                    for (int s = 0; ++s < bulge_steps;)
                    {
                        displace(w, v1, angle_a + (curve_angle * s) / bulge_steps);
                        w.cmd = SEG_LINETO;
                        push_vertex(w);
                    }
                    displace(v1, angle_b);
                    push_vertex(v1);
                }
            }
            v0.cmd = tmp_prev.cmd;
            v0.x = tmp_prev.x;
            v0.y = tmp_prev.y;
        }

        // last vertex
        if (!is_polygon)
        {
            displace(v1, angle_b);
            push_vertex(v1);
        }
        // initialization finished
        return status_ = process;
    }

    unsigned output_vertex(double* px, double* py)
    {
        if (cur_.cmd == SEG_CLOSE) *px = *py = 0.0;
        else
        {
            *px = cur_.x;
            *py = cur_.y;
        }
        return cur_.cmd;
    }

    void push_vertex(vertex2d const& v)
    {
        vertices_.push_back(v);
    }

    Geometry &              geom_;
    double                  offset_;
    double                  threshold_;
    unsigned                half_turn_segments_;
    status                  status_;
    size_t                  pos_;
    std::vector<vertex2d>   vertices_;
    vertex2d                start_;
    vertex2d                pre_first_;
    vertex2d                pre_;
    vertex2d                cur_;
};

}

#endif // MAPNIK_OFFSET_CONVERTER_HPP
