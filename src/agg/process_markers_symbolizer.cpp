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

// mapnik
#include <mapnik/graphics.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>

#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/renderer_common/process_markers_symbolizer.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>
// agg
#include "agg_basics.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_path_storage.h"
#include "agg_conv_clip_polyline.h"
#include "agg_conv_transform.h"


// boost
#include <boost/optional.hpp>

namespace mapnik {

template <typename T0, typename T1>
void agg_renderer<T0,T1>::process(markers_symbolizer const& sym,
                              feature_impl & feature,
                              proj_transform const& prj_trans)
{
    using namespace mapnik::svg;
    using color_type = agg::rgba8;
    using order_type = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>; // comp blender
    using buf_type = agg::rendering_buffer;
    using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, buf_type>;
    using renderer_base = agg::renderer_base<pixfmt_comp_type>;
    using renderer_type = agg::renderer_scanline_aa_solid<renderer_base>;
    using svg_attribute_type = agg::pod_bvector<path_attributes>;
    using svg_renderer_type = svg_renderer_agg<svg_path_adapter,
                                               svg_attribute_type,
                                               renderer_type,
                                               pixfmt_comp_type>;

    ras_ptr->reset();

    double gamma = get<value_double, keys::gamma>(sym, feature, common_.vars_);
    gamma_method_enum gamma_method = get<gamma_method_enum, keys::gamma_method>(sym, feature, common_.vars_);
    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }

    buf_type render_buffer(current_buffer_->raw_data(), current_buffer_->width(), current_buffer_->height(), current_buffer_->width() * 4);
    box2d<double> clip_box = clipping_extent(common_);

    auto renderer_context = std::tie(render_buffer,*ras_ptr,pixmap_);
    using context_type = decltype(renderer_context);
    using vector_dispatch_type = vector_markers_rasterizer_dispatch<svg_renderer_type, detector_type, context_type>;
    using raster_dispatch_type = raster_markers_rasterizer_dispatch<detector_type, context_type>;

    render_markers_symbolizer<vector_dispatch_type, raster_dispatch_type>(
        sym, feature, prj_trans, common_, clip_box, renderer_context);
}

template void agg_renderer<image_32>::process(markers_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);
}
