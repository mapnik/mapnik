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
#include <mapnik/make_unique.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
// mapnik symbolizer generics
#include <mapnik/renderer_common/process_building_symbolizer.hpp>

// stl
#include <cmath>
#include <memory>

namespace mapnik
{

template <typename T>
void cairo_renderer<T>::process(building_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e, keys::comp_op>(sym, feature, common_.vars_);
    mapnik::color fill = get<color, keys::fill>(sym, feature, common_.vars_);
    value_double opacity = get<value_double, keys::fill_opacity>(sym, feature, common_.vars_);
    value_double height = get<value_double, keys::height>(sym, feature, common_.vars_);

    context_.set_operator(comp_op);

    render_building_symbolizer::apply(
        feature, prj_trans, common_.t_, height,
        [&](path_type const& faces)
        {
            vertex_adapter va(faces);
            context_.set_color(fill.red()  * 0.8 / 255.0, fill.green() * 0.8 / 255.0,
                               fill.blue() * 0.8 / 255.0, fill.alpha() * opacity / 255.0);
            context_.add_path(va);
            context_.fill();
        },
        [&](path_type const& frame)
        {
            vertex_adapter va(frame);
            context_.set_color(fill.red()  * 0.8 / 255.0, fill.green() * 0.8/255.0,
                              fill.blue() * 0.8 / 255.0, fill.alpha() * opacity / 255.0);
            context_.set_line_width(common_.scale_factor_);
            context_.set_miter_limit(common_.scale_factor_ / 2.0);
            context_.add_path(va);
            context_.stroke();
        },
        [&](render_building_symbolizer::roof_type & roof)
        {
            context_.set_color(fill, opacity);
            context_.add_path(roof);
            context_.fill();
        });
}

template void cairo_renderer<cairo_ptr>::process(building_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
