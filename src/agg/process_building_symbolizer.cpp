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
#include <mapnik/image_any.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/renderer_common/agg_building_symbolizer_context.hpp>
#include <mapnik/renderer_common/process_building_symbolizer.hpp>

// agg
#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_color_rgba.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#pragma GCC diagnostic pop

namespace mapnik {

namespace { // (local)

struct agg_building_context
    : detail::agg_building_symbolizer_context<agg_building_context, rasterizer>
{
    using base = detail::agg_building_symbolizer_context<agg_building_context, rasterizer>;
    using pixfmt_type = agg::pixfmt_rgba32_pre;
    using renderer_base = agg::renderer_base<pixfmt_type>;
    using renderer_type = agg::renderer_scanline_aa_solid<renderer_base>;

    pixfmt_type pixf_;
    renderer_base renb_;
    renderer_type ren_;
    agg::scanline_u8 sl_;

    agg_building_context(agg::rendering_buffer & buf, rasterizer & ras)
        : base(ras), pixf_(buf), renb_(pixf_), ren_(renb_) {}

    void render_scanlines(rasterizer & ras)
    {
        agg::render_scanlines(ras, sl_, ren_);
    }

    void set_color(color const& c)
    {
        ren_.color(agg::rgba8_pre(c.red(), c.green(), c.blue(), c.alpha()));
    }
};

} // namespace mapnik::(local)

template <typename T0,typename T1>
void agg_renderer<T0,T1>::process(building_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    buffer_type & current_buffer = buffers_.top().get();
    agg::rendering_buffer buf(current_buffer.bytes(), current_buffer.width(), current_buffer.height(), current_buffer.row_size());
    agg_building_context ctx{buf, *ras_ptr};
    render_building_symbolizer rebus{sym, feature, common_};

    double gamma = get<value_double, keys::gamma>(sym, feature, common_.vars_);
    gamma_method_enum gamma_method = get<gamma_method_enum, keys::gamma_method>(sym, feature, common_.vars_);
    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }

    ctx.stroke_gen.width(rebus.stroke_width);
    ctx.stroke_gen.line_join(agg::round_join);

    rebus.setup_colors(sym, feature);
    if (rebus.has_transparent_walls())
    {
        rebus.render_back_side(true);
    }
    else
    {
        rebus.render_back_side(rebus.has_transparent_roof());
        rebus.flat_wall_tolerance(181);
    }
    rebus.apply(feature, prj_trans, ctx);
}

template void agg_renderer<image_rgba8>::process(building_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);
}
