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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/util/noncopyable.hpp>
// mapnik symbolizer generics
#include <mapnik/renderer_common/process_building_symbolizer.hpp>

namespace mapnik {

namespace { // (local)

struct cairo_building_context : util::noncopyable
{
    cairo_context & cairo_;

    explicit cairo_building_context(cairo_context & context)
        : cairo_(context) {}

    void set_color(color const& c) { cairo_.set_color(c); }

    void fill() { cairo_.fill(); }
    void fill_from(double x, double y) { cairo_.move_to(x, y); }
    void fill_to(double x, double y) { cairo_.line_to(x, y); }
    void fill_close() { cairo_.close_path(); }

    void stroke() { cairo_.stroke(); }
    void stroke_from(double x, double y) { cairo_.move_to(x, y); }
    void stroke_to(double x, double y) { cairo_.line_to(x, y); }
    void stroke_close() { cairo_.close_path(); }

    template <typename VertexSource>
    void stroke_preserve(VertexSource & vs, unsigned path_id = 0)
    {
        cairo_.add_path(vs, path_id);
        cairo_.stroke_preserve();
    }
};

} // namespace mapnik::(local)

template <typename T>
void cairo_renderer<T>::process(building_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    cairo_save_restore guard(context_);
    cairo_building_context ctx{context_};
    render_building_symbolizer rebus{sym, feature, common_};

    composite_mode_e comp_op = get<composite_mode_e, keys::comp_op>(sym, feature, common_.vars_);
    context_.set_operator(comp_op);
    context_.set_line_width(rebus.stroke_width);
    context_.set_line_join(ROUND_JOIN);

    rebus.setup_colors(sym, feature);
    rebus.render_back_side(rebus.has_transparent_fill());
    rebus.apply(feature, prj_trans, ctx);
}

template void cairo_renderer<cairo_ptr>::process(building_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
