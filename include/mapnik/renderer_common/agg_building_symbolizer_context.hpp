/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2018 Artem Pavlenko
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

#ifndef MAPNIK_RENDERER_COMMON_AGG_BUILDING_SYMBOLIZER_CONTEXT_HPP
#define MAPNIK_RENDERER_COMMON_AGG_BUILDING_SYMBOLIZER_CONTEXT_HPP

// mapnik
#include <mapnik/color.hpp>
#include <mapnik/util/noncopyable.hpp>

// agg
#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_vcgen_stroke.h"
#pragma GCC diagnostic pop

namespace mapnik { namespace detail {

template <typename Derived, typename Rasterizer>
struct agg_building_symbolizer_context : util::noncopyable
{
    agg::vcgen_stroke stroke_gen;
    Rasterizer & ras;

    explicit agg_building_symbolizer_context(Rasterizer & r)
        : ras(r) {}

    Derived & derived()
    {
        return static_cast<Derived&>(*this);
    }

    void set_color(color const& c)
    {
        // to be overridden in Derived
    }

    void fill() { derived().render_scanlines(ras); }
    void fill_from(double x, double y) { ras.move_to_d(x, y); }
    void fill_to(double x, double y) { ras.line_to_d(x, y); }
    void fill_close() { ras.close_polygon(); }

    void stroke()
    {
        stroke_gen.rewind(0);
        ras.reset();
        ras.add_path(stroke_gen);
        derived().render_scanlines(ras);
    }

    void stroke_from(double x, double y)
    {
        stroke_gen.remove_all();
        stroke_gen.add_vertex(x, y, agg::path_cmd_move_to);
    }

    void stroke_to(double x, double y)
    {
        stroke_gen.add_vertex(x, y, agg::path_cmd_line_to);
    }

    void stroke_close()
    {
        stroke_gen.add_vertex(0, 0, agg::path_cmd_end_poly |
                                    agg::path_flags_close);
    }

    template <typename VertexSource>
    void stroke_preserve(VertexSource & vs, unsigned path_id = 0)
    {
        vs.rewind(path_id);
        for (;;)
        {
            double x, y;
            unsigned cmd = vs.vertex(&x, &y);
            if (agg::is_move_to(cmd))
            {
                stroke_from(x, y);
            }
            else if (agg::is_vertex(cmd))
            {
                stroke_to(x, y);
            }
            else if (agg::is_close(cmd))
            {
                stroke_close();
                stroke();
            }
            else if (agg::is_stop(cmd))
            {
                break;
            }
        }
        // simulate the "preserve" part
        ras.reset();
        ras.add_path(vs, path_id);
    }
};

}} // namespace mapnik::detail

#endif // MAPNIK_RENDERER_COMMON_AGG_BUILDING_SYMBOLIZER_CONTEXT_HPP
