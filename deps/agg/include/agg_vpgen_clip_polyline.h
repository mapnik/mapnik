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

#ifndef AGG_VPGEN_CLIP_POLYLINE_INCLUDED
#define AGG_VPGEN_CLIP_POLYLINE_INCLUDED

#include "agg_basics.h"
#include "agg_clip_liang_barsky.h"

// https://github.com/mapnik/mapnik/issues/1860
#include <mapnik/config.hpp>

namespace agg
{

    //======================================================vpgen_clip_polyline
    //
    // See Implementation agg_vpgen_clip_polyline.cpp
    //
    class MAPNIK_DECL vpgen_clip_polyline
    {
    public:
        vpgen_clip_polyline() : 
            m_clip_box(0, 0, 1, 1),
            m_x1(0),
            m_y1(0),
            m_num_vertices(0),
            m_vertex(0),
            m_move_to(false)
        {
        }

        void clip_box(double _x1, double _y1, double _x2, double _y2)
        {
            m_clip_box.x1 = _x1;
            m_clip_box.y1 = _y1;
            m_clip_box.x2 = _x2;
            m_clip_box.y2 = _y2;
            m_clip_box.normalize();
        }

        double x1() const { return m_clip_box.x1; }
        double y1() const { return m_clip_box.y1; }
        double x2() const { return m_clip_box.x2; }
        double y2() const { return m_clip_box.y2; }

        static bool auto_close()   { return false; }
        static bool auto_unclose() { return true; }

        void     reset();
        void     move_to(double x, double y);
        void     line_to(double x, double y);
        unsigned vertex(double* x, double* y);

        // After line_to(), with nothing left to output: passes over the
        // source's next line_to vertices for as long as line_to() would
        // drop them (each beyond an edge the one before is beyond too),
        // when the source can (mapnik's vertex adapters); returns how many.
        template<class VertexSource>
        unsigned skip_beyond(const VertexSource& source)
        {
            if constexpr (requires(double* p) { source.skip_line_to([](double, double) { return true; }, p, p); })
            {
                if(m_vertex < m_num_vertices) return 0;
                unsigned f1 = clipping_flags(m_x1, m_y1, m_clip_box);
                if(f1 == 0) return 0;
                double x;
                double y;
                unsigned const n = static_cast<unsigned>(source.skip_line_to(
                    [&](double px, double py) {
                        unsigned const f2 = clipping_flags(px, py, m_clip_box);
                        if((f1 & f2) == 0) return false;
                        f1 = f2;
                        return true;
                    }, &x, &y));
                if(n != 0)
                {
                    m_vertex = 0;
                    m_num_vertices = 0;
                    m_x1 = x;
                    m_y1 = y;
                }
                return n;
            }
            else
            {
                return 0;
            }
        }

    private:
        rect_d        m_clip_box;
        double        m_x1;
        double        m_y1;
        double        m_x[2];
        double        m_y[2];
        unsigned      m_cmd[2];
        unsigned      m_num_vertices;
        unsigned      m_vertex;
        bool          m_move_to;
    };

}


#endif
