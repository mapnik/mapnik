/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

// boost


// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/renderer_common/process_polygon_symbolizer.hpp>

// agg
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_bin.h"

// stl
#include <string>
#include <map>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(polygon_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    using renderer_type = agg::renderer_scanline_bin_solid<grid_renderer_base_type>;
    using pixfmt_type = typename grid_renderer_base_type::pixfmt_type;
    using color_type = typename grid_renderer_base_type::pixfmt_type::color_type;
    using vertex_converter_type = vertex_converter<grid_rasterizer,clip_poly_tag,transform_tag,affine_transform_tag,simplify_tag,smooth_tag>;

    ras_ptr->reset();

    grid_rendering_buffer buf(pixmap_.raw_data(), common_.width_, common_.height_, common_.width_);

    render_polygon_symbolizer<vertex_converter_type>(
      sym, feature, prj_trans, common_, common_.query_extent_, *ras_ptr,
      [&](color const &, double) {
        pixfmt_type pixf(buf);

        grid_renderer_base_type renb(pixf);
        renderer_type ren(renb);

        // render id
        ren.color(color_type(feature.id()));
        agg::scanline_bin sl;
        ras_ptr->filling_rule(agg::fill_even_odd);
        agg::render_scanlines(*ras_ptr, sl, ren);

        // add feature properties to grid cache
        pixmap_.add_feature(feature);
      });
}


template void grid_renderer<grid>::process(polygon_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}

#endif
