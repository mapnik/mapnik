/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_SVG_PATH_ADAPTER_HPP
#define MAPNIK_SVG_PATH_ADAPTER_HPP

// mapnik
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/safe_cast.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_math.h"
#include "agg_array.h"
#include "agg_bezier_arc.h"
MAPNIK_DISABLE_WARNING_POP

// stl
#include <cmath>
#include <vector>

namespace mapnik {
namespace svg {

using namespace agg;

template<class VertexContainer>
class path_adapter : util::noncopyable
{
  public:
    using container_type = VertexContainer;
    using self_type = path_adapter<VertexContainer>;

    //--------------------------------------------------------------------
    path_adapter(VertexContainer& _vertices)
        : vertices_(_vertices)
        , iterator_(0)
    {}
    // void remove_all() { vertices_.remove_all(); iterator_ = 0; }
    // void free_all()   { vertices_.free_all();   iterator_ = 0; }

    // Make path functions
    //--------------------------------------------------------------------
    std::size_t start_new_path();

    void move_to(double x, double y);
    void move_rel(double dx, double dy);

    void line_to(double x, double y);
    void line_rel(double dx, double dy);

    void hline_to(double x);
    void hline_rel(double dx);

    void vline_to(double y);
    void vline_rel(double dy);

    void arc_to(double rx, double ry, double angle, bool large_arc_flag, bool sweep_flag, double x, double y);

    void arc_rel(double rx, double ry, double angle, bool large_arc_flag, bool sweep_flag, double dx, double dy);

    void curve3(double x_ctrl, double y_ctrl, double x_to, double y_to);

    void curve3_rel(double dx_ctrl, double dy_ctrl, double dx_to, double dy_to);

    void curve3(double x_to, double y_to);

    void curve3_rel(double dx_to, double dy_to);

    void curve4(double x_ctrl1, double y_ctrl1, double x_ctrl2, double y_ctrl2, double x_to, double y_to);

    void curve4_rel(double dx_ctrl1, double dy_ctrl1, double dx_ctrl2, double dy_ctrl2, double dx_to, double dy_to);

    void curve4(double x_ctrl2, double y_ctrl2, double x_to, double y_to);

    void curve4_rel(double x_ctrl2, double y_ctrl2, double x_to, double y_to);

    void end_poly(unsigned flags = path_flags_close);
    void close_polygon(unsigned flags = path_flags_none);

    // Accessors
    //--------------------------------------------------------------------
    const container_type& vertices() const { return vertices_; }
    container_type& vertices() { return vertices_; }

    std::size_t total_vertices() const;

    void rel_to_abs(double* x, double* y) const;

    unsigned last_vertex(double* x, double* y) const;
    unsigned prev_vertex(double* x, double* y) const;

    double last_x() const;
    double last_y() const;

    unsigned vertex(unsigned idx, double* x, double* y) const;
    unsigned command(unsigned idx) const;

    void modify_vertex(unsigned idx, double x, double y);
    void modify_vertex(unsigned idx, double x, double y, unsigned cmd);
    void modify_command(unsigned idx, unsigned cmd);

    // VertexSource interface
    //--------------------------------------------------------------------
    void rewind(unsigned path_id);
    unsigned vertex(double* x, double* y);

    // Arrange the orientation of a polygon, all polygons in a path,
    // or in all paths. After calling arrange_orientations() or
    // arrange_orientations_all_paths(), all the polygons will have
    // the same orientation, i.e. path_flags_cw or path_flags_ccw
    //--------------------------------------------------------------------
    unsigned arrange_polygon_orientation(unsigned start, path_flags_e orientation);
    unsigned arrange_orientations(unsigned path_id, path_flags_e orientation);
    void arrange_orientations_all_paths(path_flags_e orientation);
    void invert_polygon(unsigned start);

    // Flip all vertices horizontally or vertically,
    // between x1 and x2, or between y1 and y2 respectively
    //--------------------------------------------------------------------
    void flip_x(double x1, double x2);
    void flip_y(double y1, double y2);

    // Concatenate path. The path is added as is.
    //--------------------------------------------------------------------
    template<class VertexSource>
    void concat_path(VertexSource& vs, unsigned path_id = 0)
    {
        double x(0), y(0);
        unsigned cmd;
        vs.rewind(path_id);
        while (!is_stop(cmd = vs.vertex(&x, &y)))
        {
            vertices_.add_vertex(x, y, cmd);
        }
    }

    //--------------------------------------------------------------------
    // Join path. The path is joined with the existing one, that is,
    // it behaves as if the pen of a plotter was always down (drawing)
    template<class VertexSource>
    void join_path(VertexSource& vs, unsigned path_id = 0)
    {
        double x, y;
        unsigned cmd;
        vs.rewind(path_id);
        cmd = vs.vertex(&x, &y);
        if (!is_stop(cmd))
        {
            if (is_vertex(cmd))
            {
                double x0, y0;
                unsigned cmd0 = last_vertex(&x0, &y0);
                if (is_vertex(cmd0))
                {
                    if (calc_distance(x, y, x0, y0) > vertex_dist_epsilon)
                    {
                        if (is_move_to(cmd))
                            cmd = path_cmd_line_to;
                        vertices_.add_vertex(x, y, cmd);
                    }
                }
                else
                {
                    if (is_stop(cmd0))
                    {
                        cmd = path_cmd_move_to;
                    }
                    else
                    {
                        if (is_move_to(cmd))
                            cmd = path_cmd_line_to;
                    }
                    vertices_.add_vertex(x, y, cmd);
                }
            }
            while (!is_stop(cmd = vs.vertex(&x, &y)))
            {
                vertices_.add_vertex(x, y, is_move_to(cmd) ? unsigned(path_cmd_line_to) : cmd);
            }
        }
    }

    //--------------------------------------------------------------------
    void translate(double dx, double dy, unsigned path_id = 0);
    void translate_all_paths(double dx, double dy);

    //--------------------------------------------------------------------
    template<class Trans>
    void transform(const Trans& trans, unsigned path_id = 0)
    {
        unsigned num_ver = vertices_.total_vertices();
        for (; path_id < num_ver; path_id++)
        {
            double x, y;
            unsigned cmd = vertices_.vertex(path_id, &x, &y);
            if (is_stop(cmd))
                break;
            if (is_vertex(cmd))
            {
                trans.transform(&x, &y);
                vertices_.modify_vertex(path_id, x, y);
            }
        }
    }

    //--------------------------------------------------------------------
    template<class Trans>
    void transform_all_paths(const Trans& trans)
    {
        unsigned idx;
        unsigned num_ver = vertices_.total_vertices();
        for (idx = 0; idx < num_ver; idx++)
        {
            double x, y;
            if (is_vertex(vertices_.vertex(idx, &x, &y)))
            {
                trans.transform(&x, &y);
                vertices_.modify_vertex(idx, x, y);
            }
        }
    }

  private:
    unsigned perceive_polygon_orientation(unsigned start, unsigned end);
    void invert_polygon(unsigned start, unsigned end);

    VertexContainer& vertices_;
    unsigned iterator_;
    double start_x_;
    double start_y_;
};

//------------------------------------------------------------------------
template<class VC>
std::size_t path_adapter<VC>::start_new_path()
{
    if (!is_stop(vertices_.last_command()))
    {
        vertices_.add_vertex(0.0, 0.0, path_cmd_stop);
    }
    return vertices_.total_vertices();
}

//------------------------------------------------------------------------
template<class VC>
inline void path_adapter<VC>::rel_to_abs(double* x, double* y) const
{
    if (vertices_.total_vertices())
    {
        double x2;
        double y2;
        if (is_vertex(vertices_.last_vertex(&x2, &y2)) || !is_stop(vertices_.last_command()))
        {
            *x += x2;
            *y += y2;
        }
    }
}

//------------------------------------------------------------------------
template<class VC>
inline void path_adapter<VC>::move_to(double x, double y)
{
    start_x_ = x;
    start_y_ = y;
    vertices_.add_vertex(x, y, path_cmd_move_to);
}

//------------------------------------------------------------------------
template<class VC>
inline void path_adapter<VC>::move_rel(double dx, double dy)
{
    rel_to_abs(&dx, &dy);
    vertices_.add_vertex(dx, dy, path_cmd_move_to);
}

//------------------------------------------------------------------------
template<class VC>
inline void path_adapter<VC>::line_to(double x, double y)
{
    vertices_.add_vertex(x, y, path_cmd_line_to);
}

//------------------------------------------------------------------------
template<class VC>
inline void path_adapter<VC>::line_rel(double dx, double dy)
{
    rel_to_abs(&dx, &dy);
    vertices_.add_vertex(dx, dy, path_cmd_line_to);
}

//------------------------------------------------------------------------
template<class VC>
inline void path_adapter<VC>::hline_to(double x)
{
    vertices_.add_vertex(x, last_y(), path_cmd_line_to);
}

//------------------------------------------------------------------------
template<class VC>
inline void path_adapter<VC>::hline_rel(double dx)
{
    double dy = 0;
    rel_to_abs(&dx, &dy);
    vertices_.add_vertex(dx, dy, path_cmd_line_to);
}

//------------------------------------------------------------------------
template<class VC>
inline void path_adapter<VC>::vline_to(double y)
{
    vertices_.add_vertex(last_x(), y, path_cmd_line_to);
}

//------------------------------------------------------------------------
template<class VC>
inline void path_adapter<VC>::vline_rel(double dy)
{
    double dx = 0;
    rel_to_abs(&dx, &dy);
    vertices_.add_vertex(dx, dy, path_cmd_line_to);
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::arc_to(double rx,
                              double ry,
                              double angle,
                              bool large_arc_flag,
                              bool sweep_flag,
                              double x,
                              double y)
{
    if (vertices_.total_vertices() && is_vertex(vertices_.last_command()))
    {
        const double epsilon = 1e-30;
        double x0 = 0.0;
        double y0 = 0.0;
        vertices_.last_vertex(&x0, &y0);

        rx = std::fabs(rx);
        ry = std::fabs(ry);

        // Ensure radii are valid
        //-------------------------
        if (rx < epsilon || ry < epsilon)
        {
            line_to(x, y);
            return;
        }

        if (calc_distance(x0, y0, x, y) < epsilon)
        {
            // If the endpoints (x, y) and (x0, y0) are identical, then this
            // is equivalent to omitting the elliptical arc segment entirely.
            return;
        }
        bezier_arc_svg a(x0, y0, rx, ry, angle, large_arc_flag, sweep_flag, x, y);
        if (a.radii_ok())
        {
            join_path(a);
        }

        // We are adding an explicit line_to, even if we've already add the
        // bezier arc to the current path. This is to prevent subsequent smooth
        // bezier curves from accidentally assuming that the previous command
        // was a bezier curve as well when calculating reflection points.
        line_to(x, y);
    }
    else
    {
        move_to(x, y);
    }
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::arc_rel(double rx,
                               double ry,
                               double angle,
                               bool large_arc_flag,
                               bool sweep_flag,
                               double dx,
                               double dy)
{
    rel_to_abs(&dx, &dy);
    arc_to(rx, ry, angle, large_arc_flag, sweep_flag, dx, dy);
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::curve3(double x_ctrl, double y_ctrl, double x_to, double y_to)
{
    vertices_.add_vertex(x_ctrl, y_ctrl, path_cmd_curve3);
    vertices_.add_vertex(x_to, y_to, path_cmd_curve3);
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::curve3_rel(double dx_ctrl, double dy_ctrl, double dx_to, double dy_to)
{
    rel_to_abs(&dx_ctrl, &dy_ctrl);
    rel_to_abs(&dx_to, &dy_to);
    vertices_.add_vertex(dx_ctrl, dy_ctrl, path_cmd_curve3);
    vertices_.add_vertex(dx_to, dy_to, path_cmd_curve3);
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::curve3(double x_to, double y_to)
{
    double x0;
    double y0;
    unsigned last_cmd = last_vertex(&x0, &y0);
    if (is_vertex(last_cmd))
    {
        double x_ctrl;
        double y_ctrl;
        unsigned prev_cmd = prev_vertex(&x_ctrl, &y_ctrl);
        if (is_curve(last_cmd) && is_curve(prev_cmd))
        {
            x_ctrl = x0 + x0 - x_ctrl;
            y_ctrl = y0 + y0 - y_ctrl;
        }
        else
        {
            x_ctrl = x0;
            y_ctrl = y0;
        }
        curve3(x_ctrl, y_ctrl, x_to, y_to);
    }
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::curve3_rel(double dx_to, double dy_to)
{
    rel_to_abs(&dx_to, &dy_to);
    curve3(dx_to, dy_to);
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::curve4(double x_ctrl1, double y_ctrl1, double x_ctrl2, double y_ctrl2, double x_to, double y_to)
{
    vertices_.add_vertex(x_ctrl1, y_ctrl1, path_cmd_curve4);
    vertices_.add_vertex(x_ctrl2, y_ctrl2, path_cmd_curve4);
    vertices_.add_vertex(x_to, y_to, path_cmd_curve4);
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::curve4_rel(double dx_ctrl1,
                                  double dy_ctrl1,
                                  double dx_ctrl2,
                                  double dy_ctrl2,
                                  double dx_to,
                                  double dy_to)
{
    rel_to_abs(&dx_ctrl1, &dy_ctrl1);
    rel_to_abs(&dx_ctrl2, &dy_ctrl2);
    rel_to_abs(&dx_to, &dy_to);
    curve4(dx_ctrl1, dy_ctrl1, dx_ctrl2, dy_ctrl2, dx_to, dy_to);
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::curve4(double x_ctrl2, double y_ctrl2, double x_to, double y_to)
{
    double x0;
    double y0;
    unsigned last_cmd = last_vertex(&x0, &y0);
    if (is_vertex(last_cmd))
    {
        double x_ctrl1;
        double y_ctrl1;
        unsigned prev_cmd = prev_vertex(&x_ctrl1, &y_ctrl1);
        if (is_curve(last_cmd) && is_curve(prev_cmd))
        {
            x_ctrl1 = x0 + x0 - x_ctrl1;
            y_ctrl1 = y0 + y0 - y_ctrl1;
        }
        else
        {
            x_ctrl1 = x0;
            y_ctrl1 = y0;
        }
        curve4(x_ctrl1, y_ctrl1, x_ctrl2, y_ctrl2, x_to, y_to);
    }
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::curve4_rel(double dx_ctrl2, double dy_ctrl2, double dx_to, double dy_to)
{
    rel_to_abs(&dx_ctrl2, &dy_ctrl2);
    rel_to_abs(&dx_to, &dy_to);
    curve4(dx_ctrl2, dy_ctrl2, dx_to, dy_to);
}

//------------------------------------------------------------------------
template<class VC>
inline void path_adapter<VC>::end_poly(unsigned flags)
{
    if (is_vertex(vertices_.last_command()))
    {
        vertices_.add_vertex(start_x_, start_y_, path_cmd_end_poly | flags);
    }
}

//------------------------------------------------------------------------
template<class VC>
inline void path_adapter<VC>::close_polygon(unsigned flags)
{
    end_poly(path_flags_close | flags);
}

//------------------------------------------------------------------------
template<class VC>
inline std::size_t path_adapter<VC>::total_vertices() const
{
    return vertices_.total_vertices();
}

//------------------------------------------------------------------------
template<class VC>
inline unsigned path_adapter<VC>::last_vertex(double* x, double* y) const
{
    return vertices_.last_vertex(x, y);
}

//------------------------------------------------------------------------
template<class VC>
inline unsigned path_adapter<VC>::prev_vertex(double* x, double* y) const
{
    return vertices_.prev_vertex(x, y);
}

//------------------------------------------------------------------------
template<class VC>
inline double path_adapter<VC>::last_x() const
{
    return vertices_.last_x();
}

//------------------------------------------------------------------------
template<class VC>
inline double path_adapter<VC>::last_y() const
{
    return vertices_.last_y();
}

//------------------------------------------------------------------------
template<class VC>
inline unsigned path_adapter<VC>::vertex(unsigned idx, double* x, double* y) const
{
    return vertices_.vertex(idx, x, y);
}

//------------------------------------------------------------------------
template<class VC>
inline unsigned path_adapter<VC>::command(unsigned idx) const
{
    return vertices_.command(idx);
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::modify_vertex(unsigned idx, double x, double y)
{
    vertices_.modify_vertex(idx, x, y);
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::modify_vertex(unsigned idx, double x, double y, unsigned cmd)
{
    vertices_.modify_vertex(idx, x, y, cmd);
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::modify_command(unsigned idx, unsigned cmd)
{
    vertices_.modify_command(idx, cmd);
}

//------------------------------------------------------------------------
template<class VC>
inline void path_adapter<VC>::rewind(unsigned path_id)
{
    iterator_ = path_id;
}

//------------------------------------------------------------------------
template<class VC>
inline unsigned path_adapter<VC>::vertex(double* x, double* y)
{
    if (iterator_ >= vertices_.total_vertices())
        return path_cmd_stop;
    return vertices_.vertex(iterator_++, x, y);
}

//------------------------------------------------------------------------
template<class VC>
unsigned path_adapter<VC>::perceive_polygon_orientation(unsigned start, unsigned end)
{
    // Calculate signed area (double area to be exact)
    //---------------------
    unsigned np = end - start;
    double area = 0.0;
    unsigned i;
    for (i = 0; i < np; i++)
    {
        double x1, y1, x2, y2;
        vertices_.vertex(start + i, &x1, &y1);
        vertices_.vertex(start + (i + 1) % np, &x2, &y2);
        area += x1 * y2 - y1 * x2;
    }
    return (area < 0.0) ? path_flags_cw : path_flags_ccw;
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::invert_polygon(unsigned start, unsigned end)
{
    unsigned i;
    unsigned tmp_cmd = vertices_.command(start);

    --end; // Make "end" inclusive

    // Shift all commands to one position
    for (i = start; i < end; i++)
    {
        vertices_.modify_command(i, vertices_.command(i + 1));
    }

    // Assign starting command to the ending command
    vertices_.modify_command(end, tmp_cmd);

    // Reverse the polygon
    while (end > start)
    {
        vertices_.swap_vertices(start++, end--);
    }
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::invert_polygon(unsigned start)
{
    // Skip all non-vertices at the beginning
    while (start < vertices_.total_vertices() && !is_vertex(vertices_.command(start)))
        ++start;

    // Skip all insignificant move_to
    while (start + 1 < vertices_.total_vertices() && is_move_to(vertices_.command(start)) &&
           is_move_to(vertices_.command(start + 1)))
        ++start;

    // Find the last vertex
    unsigned end = start + 1;
    while (end < vertices_.total_vertices() && !is_next_poly(vertices_.command(end)))
        ++end;

    invert_polygon(start, end);
}

//------------------------------------------------------------------------
template<class VC>
unsigned path_adapter<VC>::arrange_polygon_orientation(unsigned start, path_flags_e orientation)
{
    if (orientation == path_flags_none)
        return start;

    // Skip all non-vertices at the beginning
    while (start < vertices_.total_vertices() && !is_vertex(vertices_.command(start)))
        ++start;

    // Skip all insignificant move_to
    while (start + 1 < vertices_.total_vertices() && is_move_to(vertices_.command(start)) &&
           is_move_to(vertices_.command(start + 1)))
        ++start;

    // Find the last vertex
    unsigned end = start + 1;
    while (end < vertices_.total_vertices() && !is_next_poly(vertices_.command(end)))
        ++end;

    if (end - start > 2)
    {
        if (perceive_polygon_orientation(start, end) != unsigned(orientation))
        {
            // Invert polygon, set orientation flag, and skip all end_poly
            invert_polygon(start, end);
            unsigned cmd;
            while (end < vertices_.total_vertices() && is_end_poly(cmd = vertices_.command(end)))
            {
                vertices_.modify_command(end++, set_orientation(cmd, orientation));
            }
        }
    }
    return end;
}

//------------------------------------------------------------------------
template<class VC>
unsigned path_adapter<VC>::arrange_orientations(unsigned start, path_flags_e orientation)
{
    if (orientation != path_flags_none)
    {
        while (start < vertices_.total_vertices())
        {
            start = arrange_polygon_orientation(start, orientation);
            if (is_stop(vertices_.command(start)))
            {
                ++start;
                break;
            }
        }
    }
    return start;
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::arrange_orientations_all_paths(path_flags_e orientation)
{
    if (orientation != path_flags_none)
    {
        unsigned start = 0;
        while (start < vertices_.total_vertices())
        {
            start = arrange_orientations(start, orientation);
        }
    }
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::flip_x(double x1, double x2)
{
    unsigned i;
    double x, y;
    for (i = 0; i < vertices_.total_vertices(); i++)
    {
        unsigned cmd = vertices_.vertex(i, &x, &y);
        if (is_vertex(cmd))
        {
            vertices_.modify_vertex(i, x2 - x + x1, y);
        }
    }
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::flip_y(double y1, double y2)
{
    unsigned i;
    double x, y;
    for (i = 0; i < vertices_.total_vertices(); i++)
    {
        unsigned cmd = vertices_.vertex(i, &x, &y);
        if (is_vertex(cmd))
        {
            vertices_.modify_vertex(i, x, y2 - y + y1);
        }
    }
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::translate(double dx, double dy, unsigned path_id)
{
    unsigned num_ver = vertices_.total_vertices();
    for (; path_id < num_ver; path_id++)
    {
        double x, y;
        unsigned cmd = vertices_.vertex(path_id, &x, &y);
        if (is_stop(cmd))
            break;
        if (is_vertex(cmd))
        {
            x += dx;
            y += dy;
            vertices_.modify_vertex(path_id, x, y);
        }
    }
}

//------------------------------------------------------------------------
template<class VC>
void path_adapter<VC>::translate_all_paths(double dx, double dy)
{
    unsigned idx;
    unsigned num_ver = vertices_.total_vertices();
    for (idx = 0; idx < num_ver; idx++)
    {
        double x, y;
        if (is_vertex(vertices_.vertex(idx, &x, &y)))
        {
            x += dx;
            y += dy;
            vertices_.modify_vertex(idx, x, y);
        }
    }
}

template<class Container>
class vertex_stl_adapter : util::noncopyable
{
  public:

    using vertex_type = typename Container::value_type;
    using value_type = typename vertex_type::value_type;

    explicit vertex_stl_adapter(Container& vertices)
        : vertices_(vertices)
    {}

    void add_vertex(double x, double y, unsigned cmd)
    {
        vertices_.push_back(vertex_type(value_type(x), value_type(y), int8u(cmd)));
    }

    void modify_vertex(unsigned idx, double x, double y)
    {
        vertex_type& v = vertices_[idx];
        v.x = value_type(x);
        v.y = value_type(y);
    }

    void modify_vertex(unsigned idx, double x, double y, unsigned cmd)
    {
        vertex_type& v = vertices_[idx];
        v.x = value_type(x);
        v.y = value_type(y);
        v.cmd = int8u(cmd);
    }

    void modify_command(unsigned idx, unsigned cmd) { vertices_[idx].cmd = int8u(cmd); }

    void swap_vertices(unsigned v1, unsigned v2)
    {
        vertex_type t = vertices_[v1];
        vertices_[v1] = vertices_[v2];
        vertices_[v2] = t;
    }

    unsigned last_command() const { return vertices_.size() ? vertices_[vertices_.size() - 1].cmd : path_cmd_stop; }

    unsigned last_vertex(double* x, double* y) const
    {
        if (vertices_.size() == 0)
        {
            *x = *y = 0.0;
            return path_cmd_stop;
        }
        return vertex(safe_cast<unsigned>(vertices_.size() - 1), x, y);
    }

    unsigned prev_vertex(double* x, double* y) const
    {
        if (vertices_.size() < 2)
        {
            *x = *y = 0.0;
            return path_cmd_stop;
        }
        return vertex(safe_cast<unsigned>(vertices_.size() - 2), x, y);
    }

    double last_x() const { return vertices_.size() ? vertices_[vertices_.size() - 1].x : 0.0; }

    double last_y() const { return vertices_.size() ? vertices_[vertices_.size() - 1].y : 0.0; }

    std::size_t total_vertices() const { return vertices_.size(); }

    unsigned vertex(unsigned idx, double* x, double* y) const
    {
        const vertex_type& v = vertices_[idx];
        *x = v.x;
        *y = v.y;
        return v.cmd;
    }

    unsigned command(unsigned idx) const { return vertices_[idx].cmd; }

  private:
    Container& vertices_;
};

using svg_path_storage = std::vector<vertex_d>;

using svg_path_adapter = path_adapter<vertex_stl_adapter<svg_path_storage>>;

} // namespace svg
} // namespace mapnik

#endif // MAPNIK_SVG_PATH_ADAPTER
