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
//
// Line dash generator
//
//----------------------------------------------------------------------------

#include <math.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include "agg_vcgen_dash.h"
#include "agg_shorten_path.h"

namespace agg
{

//------------------------------------------------------------------------
vcgen_dash::vcgen_dash() :
    m_dash_grain(HUGE_VAL),
    m_skip_limit(0.0),
    m_total_dash_len(0.0),
    m_num_dashes(0),
    m_dash_start(0.0),
    m_shorten(0.0),
    m_curr_dash_start(0.0),
    m_curr_dash(0),
    m_src_vertices(),
    m_closed(0),
    m_status(initial),
    m_src_vertex(0),
    m_cull(false),
    m_cull_x1(0.0),
    m_cull_y1(0.0),
    m_cull_x2(0.0),
    m_cull_y2(0.0),
    m_far(false),
    m_held(),
    m_flushing(false),
    m_flush(0)
{
}


//------------------------------------------------------------------------
void vcgen_dash::cull_box(double x1, double y1, double x2, double y2)
{
    m_cull = true;
    m_cull_x1 = x1;
    m_cull_y1 = y1;
    m_cull_x2 = x2;
    m_cull_y2 = y2;
}


//------------------------------------------------------------------------
// Whether the segment stays more than a pixel away from the cull box: the
// points made along it can stray from it by rounding. Anything unusual
// (non-finite or huge coordinates) counts as near.
bool vcgen_dash::far(const vertex_dist& v1, const vertex_dist& v2) const
{
    if(!(std::fabs(v1.x) < 1e12 && std::fabs(v1.y) < 1e12 &&
         std::fabs(v2.x) < 1e12 && std::fabs(v2.y) < 1e12))
    {
        return false;
    }
    return std::max(v1.x, v2.x) < m_cull_x1 - 1.0 || std::min(v1.x, v2.x) > m_cull_x2 + 1.0 ||
           std::max(v1.y, v2.y) < m_cull_y1 - 1.0 || std::min(v1.y, v2.y) > m_cull_y2 + 1.0;
}



//------------------------------------------------------------------------
void vcgen_dash::remove_all_dashes()
{
    m_total_dash_len = 0.0;
    m_num_dashes = 0;
    m_curr_dash_start = 0.0;
    m_curr_dash = 0;
    m_dash_grain = HUGE_VAL;
    m_skip_limit = 0.0;
}


//------------------------------------------------------------------------
// The largest power of two that a positive length is a multiple of (zero
// for anything else).
static double dash_grain(double len)
{
    if(!(len > 0.0 && len < HUGE_VAL)) return 0.0;
    int e;
    double const f = std::frexp(len, &e); // len = m * 2^(e - 53), m < 2^53
    std::uint64_t const m = static_cast<std::uint64_t>(std::ldexp(f, 53));
    return std::ldexp(static_cast<double>(m & (~m + 1)), e - 53);
}


//------------------------------------------------------------------------
void vcgen_dash::add_dash(double dash_len, double gap_len)
{
    if(m_num_dashes < max_dashes-1)
    {
        m_total_dash_len += dash_len + gap_len;
        m_dashes[m_num_dashes++] = dash_len;
        m_dashes[m_num_dashes++] = gap_len;
        if(dash_len != 0.0) m_dash_grain = std::min(m_dash_grain, dash_grain(dash_len));
        if(gap_len != 0.0) m_dash_grain = std::min(m_dash_grain, dash_grain(gap_len));
        // the sums of whole numbers of grains are exact below 2^53 grains
        bool const exact = m_total_dash_len > 0.0 && m_total_dash_len < std::ldexp(m_dash_grain, 52);
        m_skip_limit = exact ? std::ldexp(m_dash_grain, 53) : 0.0;
    }
}


//------------------------------------------------------------------------
// At the start of a dash or gap along a far segment, with a held move_to
// before a dash: passes over whole periods of the pattern. Along them the
// dash lengths are taken from m_curr_rest one after the other; when they are
// multiples of its ulp each subtraction is exact (the results are multiples
// of it too, below m_curr_rest), so k periods take away exactly k times the
// period, every dash and gap among them ends inside the segment, and they all
// lie along it: culling would drop them.
void vcgen_dash::skip_periods()
{
    double const period = m_total_dash_len;
    double const rest = m_curr_rest;
    // below 2^53 grains the ulp of rest is at most a grain (and the period,
    // summed in add_dash, is exact: see m_skip_limit)
    if(!(rest > 2.0 * period && rest < m_skip_limit)) return;
    double k = static_cast<double>(static_cast<std::int64_t>(rest / period));
    double left = rest - k * period;
    if(!(left > 0.0))
    {
        // the quotient was whole or rounded up to a whole number
        k -= 1.0;
        left = rest - k * period;
    }
    m_curr_rest = left;
    if(m_curr_dash & 1)
    {
        // the subpath before this gap is complete; the gap ends in a move_to
        m_held.clear();
    }
    else
    {
        // this dash starts where the last gap passed over ends
        m_held[0] = out_vertex{m_v1, m_v2, m_curr_rest, false, path_cmd_move_to};
    }
}


//------------------------------------------------------------------------
void vcgen_dash::dash_start(double ds)
{
    m_dash_start = ds;
    calc_dash_start(std::fabs(ds));
}


//------------------------------------------------------------------------
void vcgen_dash::calc_dash_start(double ds)
{
    m_curr_dash = 0;
    m_curr_dash_start = 0.0;
    while(ds > 0.0)
    {
        if(ds > m_dashes[m_curr_dash])
        {
            ds -= m_dashes[m_curr_dash];
            ++m_curr_dash;
            m_curr_dash_start = 0.0;
            if(m_curr_dash >= m_num_dashes) m_curr_dash = 0;
        }
        else
        {
            m_curr_dash_start = ds;
            ds = 0.0;
        }
    }
}


//------------------------------------------------------------------------
void vcgen_dash::remove_all()
{
    m_status = initial;
    m_src_vertices.remove_all();
    m_closed = 0;
    m_held.clear();
    m_flushing = false;
}


//------------------------------------------------------------------------
void vcgen_dash::add_vertex(double x, double y, unsigned cmd)
{
    m_status = initial;
    if(is_move_to(cmd))
    {
        m_src_vertices.modify_last(vertex_dist(x, y));
    }
    else
    {
        if(is_vertex(cmd))
        {
            m_src_vertices.add(vertex_dist(x, y));
        }
        else
        {
            m_closed = get_close_flag(cmd);
        }
    }
}


//------------------------------------------------------------------------
void vcgen_dash::rewind(unsigned)
{
    if(m_status == initial)
    {
        m_src_vertices.close(m_closed != 0);
        shorten_path(m_src_vertices, m_shorten, m_closed);
    }
    m_status = ready;
    m_src_vertex = 0;
    m_held.clear();
    m_flushing = false;
}


//------------------------------------------------------------------------
unsigned vcgen_dash::vertex(double* x, double* y)
{
    // With a cull box, each subpath (a dash, or a gap's move_to) is held back
    // while every segment of it lies along a source segment that is far from
    // the box. It is output in full once it comes near; if it ends first, it
    // is left out, as culling would drop it (a lone move_to draws nothing).
    if(m_flushing)
    {
        const out_vertex& v = m_held[m_flush++];
        point(v, x, y);
        unsigned cmd = v.cmd;
        if(m_flush == m_held.size())
        {
            m_held.clear();
            m_flushing = false;
        }
        return cmd;
    }

    unsigned cmd = path_cmd_move_to;
    while(!is_stop(cmd))
    {
        switch(m_status)
        {
        case initial:
            rewind(0);

        case ready:
            if(m_num_dashes < 2 || m_src_vertices.size() < 2)
            {
                cmd = path_cmd_stop;
                break;
            }
            m_status = polyline;
            m_src_vertex = 1;
            m_v1 = &m_src_vertices[0];
            m_v2 = &m_src_vertices[1];
            m_curr_rest = m_v1->dist;
            *x = m_v1->x;
            *y = m_v1->y;
            if(m_dash_start >= 0.0) calc_dash_start(m_dash_start);
            if(m_cull)
            {
                m_far = far(*m_v1, *m_v2);
                if(m_far)
                {
                    m_held.push_back(out_vertex{m_v1, m_v1, 0.0, true, path_cmd_move_to});
                    break;
                }
            }
            return path_cmd_move_to;

        case polyline:
        {
            if(m_far && m_curr_dash_start == 0.0 && ((m_curr_dash & 1) || m_held.size() == 1))
            {
                skip_periods();
            }

            double dash_rest = m_dashes[m_curr_dash] - m_curr_dash_start;

            unsigned cmd2 = (m_curr_dash & 1) ?
                path_cmd_move_to :
                path_cmd_line_to;

            out_vertex v{m_v1, m_v2, 0.0, false, cmd2};
            bool const far_segment = m_far;

            if(m_curr_rest > dash_rest)
            {
                m_curr_rest -= dash_rest;
                ++m_curr_dash;
                if(m_curr_dash >= m_num_dashes) m_curr_dash = 0;
                m_curr_dash_start = 0.0;
                v.rest = m_curr_rest;
            }
            else
            {
                m_curr_dash_start += m_curr_rest;
                v.at_v2 = true;
                ++m_src_vertex;
                m_v1 = m_v2;
                m_curr_rest = m_v1->dist;
                if(m_closed)
                {
                    if(m_src_vertex > m_src_vertices.size())
                    {
                        m_status = stop;
                    }
                    else
                    {
                        m_v2 = &m_src_vertices
                            [
                                (m_src_vertex >= m_src_vertices.size()) ? 0 :
                                m_src_vertex
                                ];
                    }
                }
                else
                {
                    if(m_src_vertex >= m_src_vertices.size())
                    {
                        m_status = stop;
                    }
                    else
                    {
                        m_v2 = &m_src_vertices[m_src_vertex];
                    }
                }
                if(m_cull && m_status != stop) m_far = far(*m_v1, *m_v2);
            }
            if(m_cull)
            {
                if(is_move_to(cmd2))
                {
                    m_held.clear();
                    if(far_segment)
                    {
                        m_held.push_back(v);
                        break;
                    }
                }
                else if(!m_held.empty())
                {
                    // the segment from the last vertex of the subpath to this
                    // one lies along the source segment v.v1-v.v2
                    m_held.push_back(v);
                    if(far_segment) break;
                    m_flushing = true;
                    m_flush = 1;
                    v = m_held[0];
                }
            }
            point(v, x, y);
            return v.cmd;
        }
        break;

        case stop:
            cmd = path_cmd_stop;
            m_held.clear();
            break;
        }

    }
    return path_cmd_stop;
}


}

