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
#ifndef AGG_VCGEN_DASH_INCLUDED
#define AGG_VCGEN_DASH_INCLUDED

#include "agg_basics.h"
#include "agg_vertex_sequence.h"
#include <mapnik/config.hpp>
#include <vector>

namespace agg
{

    //---------------------------------------------------------------vcgen_dash
    //
    // See Implementation agg_vcgen_dash.cpp
    //
    class MAPNIK_DECL vcgen_dash
    {
        enum max_dashes_e
        {
            max_dashes = 32
        };

        enum status_e
        {
            initial,
            ready,
            polyline,
            stop
        };

    public:
        typedef vertex_sequence<vertex_dist, 6> vertex_storage;

        vcgen_dash();

        void remove_all_dashes();
        void add_dash(double dash_len, double gap_len);
        void dash_start(double ds);

        void shorten(double s) { m_shorten = s; }
        double shorten() const { return m_shorten; }

        // Leaves out the dashes that lie entirely along segments far outside
        // the box: only for output that is culled to the box afterwards, as
        // mapnik's cull_line does (see vertex()).
        void cull_box(double x1, double y1, double x2, double y2);

        // Vertex Generator Interface
        void remove_all();
        void add_vertex(double x, double y, unsigned cmd);

        // Vertex Source Interface
        void     rewind(unsigned path_id);
        unsigned vertex(double* x, double* y);

    private:
        vcgen_dash(const vcgen_dash&);
        const vcgen_dash& operator = (const vcgen_dash&);

        void calc_dash_start(double ds);

        // A vertex of the output: the end of segment v1-v2 or the point rest
        // before it.
        struct out_vertex
        {
            const vertex_dist* v1;
            const vertex_dist* v2;
            double             rest;
            bool               at_v2;
            unsigned           cmd;
        };

        static void point(const out_vertex& v, double* x, double* y)
        {
            if(v.at_v2)
            {
                *x = v.v2->x;
                *y = v.v2->y;
            }
            else
            {
                *x = v.v2->x - (v.v2->x - v.v1->x) * v.rest / v.v1->dist;
                *y = v.v2->y - (v.v2->y - v.v1->y) * v.rest / v.v1->dist;
            }
        }

        bool far(const vertex_dist& v1, const vertex_dist& v2) const;
        void skip_periods();

        double             m_dashes[max_dashes];
        double             m_dash_grain; // the largest power of two all dash lengths are multiples of
        double             m_skip_limit; // skip_periods() is exact below it (zero: never)
        double             m_total_dash_len;
        unsigned           m_num_dashes;
        double             m_dash_start;
        double             m_shorten;
        double             m_curr_dash_start;
        unsigned           m_curr_dash;
        double             m_curr_rest;
        const vertex_dist* m_v1;
        const vertex_dist* m_v2;

        vertex_storage m_src_vertices;
        unsigned       m_closed;
        status_e       m_status;
        unsigned       m_src_vertex;

        bool                     m_cull;
        double                   m_cull_x1;
        double                   m_cull_y1;
        double                   m_cull_x2;
        double                   m_cull_y2;
        bool                     m_far; // the segment m_v1-m_v2 is far from the cull box
        // the subpath being generated, held back while it lies along far segments
        std::vector<out_vertex>  m_held;
        bool                     m_flushing; // outputting m_held
        unsigned                 m_flush;    // the next of m_held to output
    };


}

#endif
