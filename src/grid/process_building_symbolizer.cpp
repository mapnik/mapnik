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

#if defined(GRID_RENDERER)

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/renderer_common/agg_building_symbolizer_context.hpp>
#include <mapnik/renderer_common/process_building_symbolizer.hpp>

// agg
#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_bin.h"
#pragma GCC diagnostic pop

namespace mapnik {

namespace { // (local)

struct grid_building_context
    : detail::agg_building_symbolizer_context<grid_building_context, grid_rasterizer>
{
    using base = detail::agg_building_symbolizer_context<grid_building_context, grid_rasterizer>;
    using renderer_base = grid_renderer_base_type;
    using renderer_type = agg::renderer_scanline_bin_solid<renderer_base>;
    using pixfmt_type = typename renderer_base::pixfmt_type;
    using color_type = typename renderer_base::pixfmt_type::color_type;

    pixfmt_type pixf_;
    renderer_base renb_;
    renderer_type ren_;
    agg::scanline_bin sl_;

    grid_building_context(grid_rendering_buffer & buf, grid_rasterizer & ras)
        : base(ras), pixf_(buf), renb_(pixf_), ren_(renb_) {}

    void render_scanlines(grid_rasterizer & ras)
    {
        agg::render_scanlines(ras, sl_, ren_);
    }

    void set_feature(value_integer id)
    {
        ren_.color(color_type(id));
    }
};

} // namespace mapnik::(local)

template <typename T>
void grid_renderer<T>::process(building_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    grid_rendering_buffer buf(pixmap_.raw_data(), common_.width_, common_.height_, common_.width_);
    grid_building_context ctx{buf, *ras_ptr};
    render_building_symbolizer rebus{sym, feature, common_};

    ctx.stroke_gen.width(rebus.stroke_width);
    ctx.stroke_gen.line_join(agg::round_join);
    ctx.set_feature(feature.id());

    rebus.render_back_side(false);
    rebus.flat_wall_tolerance(181);
    rebus.apply(feature, prj_trans, ctx);

    pixmap_.add_feature(feature);
}

template void grid_renderer<grid>::process(building_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}

#endif
