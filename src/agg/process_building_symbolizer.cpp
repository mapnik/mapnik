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

// mapnik
#include <mapnik/make_unique.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/renderer_common/process_building_symbolizer.hpp>

// stl
#include <deque>
#include <memory>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_color_rgba.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_conv_stroke.h"
#pragma GCC diagnostic pop

namespace mapnik
{

template <typename T0,typename T1>
void agg_renderer<T0,T1>::process(building_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    using ren_base = agg::renderer_base<agg::pixfmt_rgba32_pre>;
    using renderer = agg::renderer_scanline_aa_solid<ren_base>;

    buffer_type & current_buffer = buffers_.top().get();
    agg::rendering_buffer buf(current_buffer.bytes(), current_buffer.width(), current_buffer.height(), current_buffer.row_size());
    agg::pixfmt_rgba32_pre pixf(buf);
    ren_base renb(pixf);
    renderer ren(renb);
    agg::scanline_u8 sl;
    render_building_symbolizer rebus{sym, feature, common_};

    double gamma = get<value_double, keys::gamma>(sym, feature, common_.vars_);
    gamma_method_enum gamma_method = get<gamma_method_enum, keys::gamma_method>(sym, feature, common_.vars_);
    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }

    rebus.setup_colors(sym, feature);

    rebus.apply(
        feature, prj_trans,
        [&](path_type const& faces, color const& c)
        {
            vertex_adapter va(faces);
            ras_ptr->reset();
            ras_ptr->add_path(va);
            ren.color(agg::rgba8_pre(c.red(), c.green(), c.blue(), c.alpha()));
            agg::render_scanlines(*ras_ptr, sl, ren);
        },
        [&](path_type const& frame, color const& c)
        {
            vertex_adapter va(frame);
            agg::conv_stroke<vertex_adapter> stroke(va);
            stroke.width(rebus.stroke_width);
            stroke.miter_limit(1.0);
            ras_ptr->reset();
            ras_ptr->add_path(stroke);
            ren.color(agg::rgba8_pre(c.red(), c.green(), c.blue(), c.alpha()));
            agg::render_scanlines(*ras_ptr, sl, ren);
        },
        [&](render_building_symbolizer::roof_type & roof, color const& c)
        {
            ras_ptr->reset();
            ras_ptr->add_path(roof);
            ren.color(agg::rgba8_pre(c.red(), c.green(), c.blue(), c.alpha()));
            agg::render_scanlines(*ras_ptr, sl, ren);
        });
}

template void agg_renderer<image_rgba8>::process(building_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);
}
