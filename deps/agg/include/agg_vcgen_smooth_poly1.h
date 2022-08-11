//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------

#ifndef AGG_VCGEN_SMOOTH_POLY1_INCLUDED
#define AGG_VCGEN_SMOOTH_POLY1_INCLUDED

#include "agg_basics.h"
#include "agg_vertex_sequence.h"

namespace agg {

//======================================================vcgen_smooth
//
// Smooth polygon generator
//
//------------------------------------------------------------------------
template<class Calculate>
class vcgen_smooth
{
    enum status_e { initial, ready, polygon, ctrl_b, ctrl_e, ctrl1, ctrl2, end_poly, stop };

  public:
    typedef vertex_sequence<vertex_dist, 6> vertex_storage;

    vcgen_smooth()
        : m_src_vertices()
        , m_smooth_value(0.5)
        , m_closed(0)
        , m_status(initial)
        , m_src_vertex(0)
    {}

    vcgen_smooth(vcgen_smooth&&) = default;

    void smooth_value(double v) { m_smooth_value = v * 0.5; }
    double smooth_value() const { return m_smooth_value * 2.0; }

    // Vertex Generator Interface
    void remove_all()
    {
        m_src_vertices.remove_all();
        m_closed = 0;
        m_status = initial;
    }

    void add_vertex(double x, double y, unsigned cmd)
    {
        m_status = initial;
        if (is_move_to(cmd))
        {
            m_src_vertices.modify_last(vertex_dist(x, y));
        }
        else
        {
            if (is_vertex(cmd))
            {
                m_src_vertices.add(vertex_dist(x, y));
            }
            else
            {
                m_closed = get_close_flag(cmd);
            }
        }
    }

    // Vertex Source Interface
    void rewind(unsigned)
    {
        if (m_status == initial)
        {
            m_src_vertices.close(m_closed != 0);
        }
        m_status = ready;
        m_src_vertex = 0;
    }

    unsigned vertex(double* x, double* y)
    {
        unsigned cmd = path_cmd_line_to;
        while (!is_stop(cmd))
        {
            switch (m_status)
            {
                case initial:
                    rewind(0);

                case ready:
                    if (m_src_vertices.size() < 2)
                    {
                        cmd = path_cmd_stop;
                        break;
                    }

                    if (m_src_vertices.size() == 2)
                    {
                        *x = m_src_vertices[m_src_vertex].x;
                        *y = m_src_vertices[m_src_vertex].y;
                        m_src_vertex++;
                        if (m_src_vertex == 1)
                            return path_cmd_move_to;
                        if (m_src_vertex == 2)
                            return path_cmd_line_to;
                        cmd = path_cmd_stop;
                        break;
                    }

                    cmd = path_cmd_move_to;
                    m_status = polygon;
                    m_src_vertex = 0;

                case polygon:
                    if (m_closed)
                    {
                        if (m_src_vertex >= m_src_vertices.size())
                        {
                            *x = m_src_vertices[0].x;
                            *y = m_src_vertices[0].y;
                            m_status = end_poly;
                            return path_cmd_curve4;
                        }
                    }
                    else
                    {
                        if (m_src_vertex >= m_src_vertices.size() - 1)
                        {
                            *x = m_src_vertices[m_src_vertices.size() - 1].x;
                            *y = m_src_vertices[m_src_vertices.size() - 1].y;
                            m_status = end_poly;
                            return path_cmd_curve3;
                        }
                    }

                    std::tie(m_ctrl1, m_ctrl2) = Calculate::apply(m_src_vertices.prev(m_src_vertex),
                                                                  m_src_vertices.curr(m_src_vertex),
                                                                  m_src_vertices.next(m_src_vertex),
                                                                  m_src_vertices.next(m_src_vertex + 1),
                                                                  m_smooth_value);

                    *x = m_src_vertices[m_src_vertex].x;
                    *y = m_src_vertices[m_src_vertex].y;
                    m_src_vertex++;

                    if (m_closed)
                    {
                        m_status = ctrl1;
                        return ((m_src_vertex == 1) ? path_cmd_move_to : path_cmd_curve4);
                    }
                    else
                    {
                        if (m_src_vertex == 1)
                        {
                            m_status = ctrl_b;
                            return path_cmd_move_to;
                        }
                        if (m_src_vertex >= m_src_vertices.size() - 1)
                        {
                            m_status = ctrl_e;
                            return path_cmd_curve3;
                        }
                        m_status = ctrl1;
                        return path_cmd_curve4;
                    }
                    break;

                case ctrl_b:
                    *x = m_ctrl2.x;
                    *y = m_ctrl2.y;
                    m_status = polygon;
                    return path_cmd_curve3;

                case ctrl_e:
                    *x = m_ctrl1.x;
                    *y = m_ctrl1.y;
                    m_status = polygon;
                    return path_cmd_curve3;

                case ctrl1:
                    *x = m_ctrl1.x;
                    *y = m_ctrl1.y;
                    m_status = ctrl2;
                    return path_cmd_curve4;

                case ctrl2:
                    *x = m_ctrl2.x;
                    *y = m_ctrl2.y;
                    m_status = polygon;
                    return path_cmd_curve4;

                case end_poly:
                    m_status = stop;
                    return path_cmd_end_poly | m_closed;

                case stop:
                    return path_cmd_stop;
            }
        }
        return cmd;
    }

  private:
    vcgen_smooth(const vcgen_smooth&);
    const vcgen_smooth& operator=(const vcgen_smooth&);

    vertex_storage m_src_vertices;
    double m_smooth_value;
    unsigned m_closed;
    status_e m_status;
    unsigned m_src_vertex;
    point_d m_ctrl1;
    point_d m_ctrl2;
};

struct calculate_poly1
{
    using result_type = std::pair<point_d, point_d>;

    static result_type apply(const vertex_dist& v0,
                             const vertex_dist& v1,
                             const vertex_dist& v2,
                             const vertex_dist& v3,
                             double smooth_value)
    {
        double k1 = v0.dist / (v0.dist + v1.dist);
        double k2 = v1.dist / (v1.dist + v2.dist);

        double xm1 = v0.x + (v2.x - v0.x) * k1;
        double ym1 = v0.y + (v2.y - v0.y) * k1;
        double xm2 = v1.x + (v3.x - v1.x) * k2;
        double ym2 = v1.y + (v3.y - v1.y) * k2;

        return {{v1.x + smooth_value * (v2.x - xm1), v1.y + smooth_value * (v2.y - ym1)},
                {v2.x + smooth_value * (v1.x - xm2), v2.y + smooth_value * (v1.y - ym2)}};
    }
};

using vcgen_smooth_poly1 = vcgen_smooth<calculate_poly1>;

} // namespace agg

#endif
