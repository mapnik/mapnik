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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/label_collision_detector.hpp>

namespace mapnik {

namespace {

// special implementation of the box drawing so that it's pixel-aligned
inline void render_debug_box(cairo_context& context, box2d<double> const& b)
{
    cairo_save_restore guard(context);
    double minx = std::floor(b.minx()) + 0.5;
    double miny = std::floor(b.miny()) + 0.5;
    double maxx = std::floor(b.maxx()) + 0.5;
    double maxy = std::floor(b.maxy()) + 0.5;
    context.move_to(minx, miny);
    context.line_to(minx, maxy);
    context.line_to(maxx, maxy);
    context.line_to(maxx, miny);
    context.close_path();
    context.stroke();
}

template<typename Context>
struct apply_vertex_mode
{
    apply_vertex_mode(Context& context, view_transform const& t, proj_transform const& prj_trans)
        : context_(context)
        , t_(t)
        , prj_trans_(prj_trans)
    {}

    template<typename Adapter>
    void operator()(Adapter const& va) const
    {
        double x;
        double y;
        double z = 0;
        va.rewind(0);
        unsigned cmd = SEG_END;
        while ((cmd = va.vertex(&x, &y)) != mapnik::SEG_END)
        {
            if (cmd == SEG_CLOSE)
                continue;
            prj_trans_.backward(x, y, z);
            t_.forward(&x, &y);
            context_.move_to(std::floor(x) - 0.5, std::floor(y) + 0.5);
            context_.line_to(std::floor(x) + 1.5, std::floor(y) + 0.5);
            context_.move_to(std::floor(x) + 0.5, std::floor(y) - 0.5);
            context_.line_to(std::floor(x) + 0.5, std::floor(y) + 1.5);
            context_.stroke();
        }
    }

    Context& context_;
    view_transform const& t_;
    proj_transform const& prj_trans_;
};

} // anonymous namespace

template<typename T>
void cairo_renderer<T>::process(debug_symbolizer const& sym,
                                mapnik::feature_impl& feature,
                                proj_transform const& prj_trans)
{
    cairo_save_restore guard(context_);

    debug_symbolizer_mode_enum mode =
      get<debug_symbolizer_mode_enum>(sym,
                                      keys::mode,
                                      feature,
                                      common_.vars_,
                                      debug_symbolizer_mode_enum::DEBUG_SYM_MODE_COLLISION);

    context_.set_operator(src_over);
    context_.set_color(mapnik::color(255, 0, 0), 1.0);
    context_.set_line_join(line_join_enum::MITER_JOIN);
    context_.set_line_cap(line_cap_enum::BUTT_CAP);
    context_.set_miter_limit(4.0);
    context_.set_line_width(1.0);

    if (mode == debug_symbolizer_mode_enum::DEBUG_SYM_MODE_COLLISION)
    {
        for (auto& n : *common_.detector_)
        {
            render_debug_box(context_, n.get().box);
        }
    }
    else if (mode == debug_symbolizer_mode_enum::DEBUG_SYM_MODE_VERTEX)
    {
        using apply_vertex_mode = apply_vertex_mode<cairo_context>;
        apply_vertex_mode apply(context_, common_.t_, prj_trans);
        util::apply_visitor(geometry::vertex_processor<apply_vertex_mode>(apply), feature.get_geometry());
    }
}

template void cairo_renderer<cairo_ptr>::process(debug_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik

#endif // HAVE_CAIRO
