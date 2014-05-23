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
    typedef agg::rgba8 color_type;
    typedef agg::order_rgba order_type;
    typedef agg::comp_op_adaptor_rgba_pre<color_type, order_type> blender_type; // comp blender
    typedef agg::rendering_buffer buf_type;
    typedef agg::pixfmt_custom_blend_rgba<blender_type, buf_type> pixfmt_comp_type;
    typedef agg::renderer_base<pixfmt_comp_type> renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_type;
    typedef agg::pod_bvector<path_attributes> svg_attribute_type;
    typedef svg_renderer_agg<svg_path_adapter,
                             svg_attribute_type,
                             renderer_type,
                             pixfmt_comp_type > svg_renderer_type;
    typedef vector_markers_rasterizer_dispatch<buf_type,
                                               svg_renderer_type,
                                               rasterizer,
                                               detector_type > vector_dispatch_type;
    typedef raster_markers_rasterizer_dispatch<buf_type,rasterizer, detector_type> raster_dispatch_type;

    double gamma = get<value_double>(sym, keys::gamma, feature, common_.vars_, 1.0);
    gamma_method_enum gamma_method = get<gamma_method_enum>(sym, keys::gamma_method, feature, common_.vars_, GAMMA_POWER);
    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }
    
    buf_type render_buffer(current_buffer_->raw_data(), current_buffer_->width(), current_buffer_->height(), current_buffer_->width() * 4);

    ras_ptr->reset();
    box2d<double> clip_box = clipping_extent();

    render_markers_symbolizer(
        sym, feature, prj_trans, common_, clip_box,
        [&](svg_path_adapter &path, svg_attribute_type const &attr, svg_storage_type &,
            box2d<double> const &bbox, agg::trans_affine const &tr,
            bool snap_pixels) -> vector_dispatch_type {
            coord2d center = bbox.center();
            agg::trans_affine_translation recenter(-center.x, -center.y);
            agg::trans_affine marker_trans = recenter * tr;
            return vector_dispatch_type(render_buffer,
                                        path, attr,
                                        *ras_ptr,
                                        bbox,
                                        marker_trans,
                                        sym,
                                        *common_.detector_,
                                        feature, common_.vars_,
                                        common_.scale_factor_,
                                        snap_pixels);
        },
        [&](image_data_32 const &marker, agg::trans_affine const &tr,
            box2d<double> const &bbox) -> raster_dispatch_type {
            coord2d center = bbox.center();
            agg::trans_affine_translation recenter(-center.x, -center.y);
            agg::trans_affine marker_trans = recenter * tr;
            return raster_dispatch_type(render_buffer,
                                        *ras_ptr,
                                        marker,
                                        marker_trans,
                                        sym,
                                        *common_.detector_,
                                        feature, common_.vars_,
                                        common_.scale_factor_,
                                        true /*snap rasters no matter what*/);
        });
}

template void agg_renderer<image_32>::process(markers_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);
}
