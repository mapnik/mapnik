/*******************************************************************************
 *                                                                              *
 * Author    :  Angus Johnson                                                   *
 * Version   :  1.1                                                             *
 * Date      :  4 April 2011                                                    *
 * Website   :  http://www.angusj.com                                           *
 * Copyright :  Angus Johnson 2010-2011                                         *
 *                                                                              *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *                                                                              *
 *******************************************************************************/

// based on adapted agg_conv_clipper.h

#ifndef AGG_CONV_OFFSET_INCLUDED
#define AGG_CONV_OFFSET_INCLUDED

#include <cmath>
#include "agg_basics.h"
#include "agg_array.h"
#include "clipper.hpp"

namespace agg {

template<class VSA>
class conv_offset
{
    enum status { status_move_to, status_line_to, status_stop };
    typedef VSA source_a_type;
    typedef conv_offset<source_a_type> self_type;

  private:
    source_a_type* m_src_a;
    double m_offset;
    status m_status;
    int m_vertex;
    int m_contour;
    int m_scaling_factor;
    pod_bvector<ClipperLib::IntPoint, 8> m_vertex_accumulator;
    ClipperLib::Paths m_poly_a;
    ClipperLib::Paths m_result;
    ClipperLib::ClipperOffset m_clipper_offset;

    int Round(double val)
    {
        if ((val < 0))
            return (int)(val - 0.5);
        else
            return (int)(val + 0.5);
    }

  public:
    conv_offset(source_a_type& a, double offset = 0.0, int scaling_factor = 0)
        : m_src_a(&a)
        , m_offset(offset)
        , m_status(status_move_to)
        , m_vertex(-1)
        , m_contour(-1)
    {
        m_scaling_factor = std::max(std::min(scaling_factor, 6), 0);
        m_scaling_factor = Round(std::pow((double)10, m_scaling_factor));
    }

    ~conv_offset() {}

    void set_offset(double offset) { m_offset = offset; }
    unsigned type() const { return static_cast<unsigned>(m_src_a->type()); }

    double get_offset() const { return m_offset; }

    void rewind(unsigned path_id);
    unsigned vertex(double* x, double* y);

    bool next_contour();
    bool next_vertex(double* x, double* y);
    void start_extracting();
    void add_vertex_(double& x, double& y);
    void end_contour(ClipperLib::Paths& p);

    template<class VS>
    void add(VS& src, ClipperLib::Paths& p)
    {
        unsigned cmd;
        double x;
        double y;
        double start_x;
        double start_y;
        bool starting_first_line;

        start_x = 0.0;
        start_y = 0.0;
        starting_first_line = true;
        p.resize(0);

        cmd = src->vertex(&x, &y);
        while (!is_stop(cmd))
        {
            if (is_vertex(cmd))
            {
                if (is_move_to(cmd))
                {
                    if (!starting_first_line)
                        end_contour(p);
                    start_x = x;
                    start_y = y;
                }
                add_vertex_(x, y);
                starting_first_line = false;
            }
            else if (is_end_poly(cmd))
            {
                if (!starting_first_line && is_closed(cmd))
                    add_vertex_(start_x, start_y);
            }
            cmd = src->vertex(&x, &y);
        }
        end_contour(p);
    }
};

//------------------------------------------------------------------------

template<class VSA>
void conv_offset<VSA>::start_extracting()
{
    m_status = status_move_to;
    m_contour = -1;
    m_vertex = -1;
}
//------------------------------------------------------------------------------

template<class VSA>
void conv_offset<VSA>::rewind(unsigned path_id)
{
    m_src_a->rewind(path_id);
    // m_src_b->rewind( path_id );

    add(m_src_a, m_poly_a);
    // add( m_src_b , m_poly_b );
    m_result.resize(0);
    m_clipper_offset.Clear();
    m_clipper_offset.AddPaths(m_poly_a, ClipperLib::jtMiter, ClipperLib::etOpenButt); // ClosedLine);//Polygon);
    m_clipper_offset.Execute(m_result, m_offset * m_scaling_factor);
    start_extracting();
}
//------------------------------------------------------------------------------

template<class VSA>
void conv_offset<VSA>::end_contour(ClipperLib::Paths& p)
{
    unsigned i, len;

    if (m_vertex_accumulator.size() < 3)
        return;
    len = p.size();
    p.resize(len + 1);
    p[len].resize(m_vertex_accumulator.size());
    for (i = 0; i < m_vertex_accumulator.size(); i++)
        p[len][i] = m_vertex_accumulator[i];
    m_vertex_accumulator.remove_all();
}
//------------------------------------------------------------------------------

template<class VSA>
void conv_offset<VSA>::add_vertex_(double& x, double& y)
{
    ClipperLib::IntPoint v;

    v.X = Round(x * m_scaling_factor);
    v.Y = Round(y * m_scaling_factor);
    m_vertex_accumulator.add(v);
}
//------------------------------------------------------------------------------

template<class VSA>
bool conv_offset<VSA>::next_contour()
{
    m_contour++;
    if (m_contour >= (int)m_result.size())
        return false;
    m_vertex = -1;
    return true;
}
//------------------------------------------------------------------------------

template<class VSA>
bool conv_offset<VSA>::next_vertex(double* x, double* y)
{
    m_vertex++;
    if (m_vertex >= (int)m_result[m_contour].size())
        return false;
    *x = (double)m_result[m_contour][m_vertex].X / m_scaling_factor;
    *y = (double)m_result[m_contour][m_vertex].Y / m_scaling_factor;
    return true;
}
//------------------------------------------------------------------------------

template<class VSA>
unsigned conv_offset<VSA>::vertex(double* x, double* y)
{
    if (m_status == status_move_to)
    {
        if (next_contour())
        {
            if (next_vertex(x, y))
            {
                m_status = status_line_to;
                return path_cmd_move_to;
            }
            else
            {
                m_status = status_stop;
                return path_cmd_end_poly | path_flags_close;
            }
        }
        else
            return path_cmd_stop;
    }
    else
    {
        if (next_vertex(x, y))
        {
            return path_cmd_line_to;
        }
        else
        {
            m_status = status_move_to;
            return path_cmd_end_poly | path_flags_close;
        }
    }
}
//------------------------------------------------------------------------------

} // namespace agg
#endif // AGG_CONV_OFFSET_INCLUDED
