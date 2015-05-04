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

// mapnik
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>

#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/agg_render_marker.hpp>
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

namespace detail {

template <typename SvgRenderer, typename Detector, typename RendererContext>
struct vector_markers_rasterizer_dispatch : public vector_markers_dispatch<Detector>
{
    using renderer_base = typename SvgRenderer::renderer_base;
    using vertex_source_type = typename SvgRenderer::vertex_source_type;
    using attribute_source_type = typename SvgRenderer::attribute_source_type;
    using pixfmt_type = typename renderer_base::pixfmt_type;

    using BufferType = typename std::tuple_element<0,RendererContext>::type;
    using RasterizerType = typename std::tuple_element<1,RendererContext>::type;

    vector_markers_rasterizer_dispatch(svg_path_ptr const& src,
                                       vertex_source_type & path,
                                       svg_attribute_type const& attrs,
                                       agg::trans_affine const& marker_trans,
                                       symbolizer_base const& sym,
                                       Detector & detector,
                                       double scale_factor,
                                       feature_impl & feature,
                                       attributes const& vars,
                                       bool snap_to_pixels,
                                       RendererContext const& renderer_context)
: vector_markers_dispatch<Detector>(src, marker_trans, sym, detector, scale_factor, feature, vars),
        buf_(std::get<0>(renderer_context)),
        pixf_(buf_),
        renb_(pixf_),
        svg_renderer_(path, attrs),
        ras_(std::get<1>(renderer_context)),
        snap_to_pixels_(snap_to_pixels)
    {
        pixf_.comp_op(static_cast<agg::comp_op_e>(get<composite_mode_e, keys::comp_op>(sym, feature, vars)));
    }

    ~vector_markers_rasterizer_dispatch() {}

    void render_marker(agg::trans_affine const& marker_tr, double opacity)
    {
        render_vector_marker(svg_renderer_, ras_, renb_, this->src_->bounding_box(),
                             marker_tr, opacity, snap_to_pixels_);
    }

private:
    BufferType & buf_;
    pixfmt_type pixf_;
    renderer_base renb_;
    SvgRenderer svg_renderer_;
    RasterizerType & ras_;
    bool snap_to_pixels_;
};

template <typename Detector, typename RendererContext>
struct raster_markers_rasterizer_dispatch : public raster_markers_dispatch<Detector>
{
    using BufferType = typename std::remove_reference<typename std::tuple_element<0,RendererContext>::type>::type;
    using RasterizerType = typename std::tuple_element<1,RendererContext>::type;

    using color_type = agg::rgba8;
    using order_type = agg::order_rgba;
    using pixel_type = agg::pixel32_type;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>; // comp blender
    using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, BufferType>;
    using renderer_base = agg::renderer_base<pixfmt_comp_type>;

    raster_markers_rasterizer_dispatch(image_rgba8 const& src,
                                       agg::trans_affine const& marker_trans,
                                       symbolizer_base const& sym,
                                       Detector & detector,
                                       double scale_factor,
                                       feature_impl & feature,
                                       attributes const& vars,
                                       RendererContext const& renderer_context,
                                       bool snap_to_pixels = false)
    : raster_markers_dispatch<Detector>(src, marker_trans, sym, detector, scale_factor, feature, vars),
        buf_(std::get<0>(renderer_context)),
        pixf_(buf_),
        renb_(pixf_),
        ras_(std::get<1>(renderer_context)),
        snap_to_pixels_(snap_to_pixels)
    {
        pixf_.comp_op(static_cast<agg::comp_op_e>(get<composite_mode_e, keys::comp_op>(sym, feature, vars)));
    }

    ~raster_markers_rasterizer_dispatch() {}

    void render_marker(agg::trans_affine const& marker_tr, double opacity)
    {
        // In the long term this should be a visitor pattern based on the type of render this->src_ provided that converts
        // the destination pixel type required.
        render_raster_marker(renb_, ras_, this->src_, marker_tr, opacity, this->scale_factor_, snap_to_pixels_);
    }

private:
    BufferType & buf_;
    pixfmt_comp_type pixf_;
    renderer_base renb_;
    RasterizerType & ras_;
    bool snap_to_pixels_;
};

}

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

    buf_type render_buffer(current_buffer_->bytes(), current_buffer_->width(), current_buffer_->height(), current_buffer_->row_size());
    box2d<double> clip_box = clipping_extent(common_);

    auto renderer_context = std::tie(render_buffer,*ras_ptr,pixmap_);
    using context_type = decltype(renderer_context);
    using vector_dispatch_type = detail::vector_markers_rasterizer_dispatch<svg_renderer_type, detector_type, context_type>;
    using raster_dispatch_type = detail::raster_markers_rasterizer_dispatch<detector_type, context_type>;

    render_markers_symbolizer<vector_dispatch_type, raster_dispatch_type>(
        sym, feature, prj_trans, common_, clip_box, renderer_context);
}

template void agg_renderer<image_rgba8>::process(markers_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);
}
