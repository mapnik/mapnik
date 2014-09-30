/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/proj_transform.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/label_collision_detector.hpp>

namespace mapnik
{

namespace {

// special implementation of the box drawing so that it's pixel-aligned
inline void render_debug_box(cairo_context &context, box2d<double> const& b)
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

} // anonymous namespace

template <typename T>
void cairo_renderer<T>::process(debug_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    using detector_type = label_collision_detector4;
    cairo_save_restore guard(context_);

    debug_symbolizer_mode_enum mode = get<debug_symbolizer_mode_enum>(sym, keys::mode, feature, common_.vars_, DEBUG_SYM_MODE_COLLISION);

    context_.set_operator(src_over);
    context_.set_color(mapnik::color(255, 0, 0), 1.0);
    context_.set_line_join(MITER_JOIN);
    context_.set_line_cap(BUTT_CAP);
    context_.set_miter_limit(4.0);
    context_.set_line_width(1.0);

    if (mode == DEBUG_SYM_MODE_COLLISION)
    {
        typename detector_type::query_iterator itr = common_.detector_->begin();
        typename detector_type::query_iterator end = common_.detector_->end();
        for ( ;itr!=end; ++itr)
        {
            render_debug_box(context_, itr->box);
        }
    }
    else if (mode == DEBUG_SYM_MODE_VERTEX)
    {
        for (auto const& geom : feature.paths())
        {
            double x;
            double y;
            double z = 0;
            geom.rewind(0);
            unsigned cmd = 1;
            while ((cmd = geom.vertex(&x, &y)) != mapnik::SEG_END)
            {
                if (cmd == SEG_CLOSE) continue;
                prj_trans.backward(x,y,z);
                common_.t_.forward(&x,&y);
                context_.move_to(std::floor(x) - 0.5, std::floor(y) + 0.5);
                context_.line_to(std::floor(x) + 1.5, std::floor(y) + 0.5);
                context_.move_to(std::floor(x) + 0.5, std::floor(y) - 0.5);
                context_.line_to(std::floor(x) + 0.5, std::floor(y) + 1.5);
                context_.stroke();
            }
        }
    }
}

template void cairo_renderer<cairo_ptr>::process(debug_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
