/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/agg_render_marker.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>
#include <mapnik/renderer_common/render_markers_symbolizer.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_path_storage.h"
#include "agg_conv_transform.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

namespace detail {

template<typename SvgRenderer, typename BufferType, typename RasterizerType>
struct agg_markers_renderer_context : markers_renderer_context
{
    using renderer_base = typename SvgRenderer::renderer_base;
    using vertex_source_type = typename SvgRenderer::vertex_source_type;
    using pixfmt_type = typename renderer_base::pixfmt_type;

    agg_markers_renderer_context(symbolizer_base const& sym,
                                 feature_impl const& feature,
                                 attributes const& vars,
                                 BufferType& buf,
                                 RasterizerType& ras)
        : buf_(buf)
        , pixf_(buf_)
        , renb_(pixf_)
        , ras_(ras)
    {
        auto comp_op = get<composite_mode_e, keys::comp_op>(sym, feature, vars);
        pixf_.comp_op(static_cast<agg::comp_op_e>(comp_op));
    }

    virtual void render_marker(svg_path_ptr const& src,
                               svg_path_adapter& path,
                               svg::group const& group_attrs,
                               markers_dispatch_params const& params,
                               agg::trans_affine const& marker_tr)
    {
        SvgRenderer svg_renderer(path, group_attrs);
        render_vector_marker(svg_renderer,
                             ras_,
                             renb_,
                             src->bounding_box(),
                             marker_tr,
                             params.opacity,
                             params.snap_to_pixels);
    }

    virtual void
      render_marker(image_rgba8 const& src, markers_dispatch_params const& params, agg::trans_affine const& marker_tr)
    {
        // In the long term this should be a visitor pattern based on the type of
        // render src provided that converts the destination pixel type required.
        render_raster_marker(renb_, ras_, src, marker_tr, params.opacity, params.scale_factor, params.snap_to_pixels);
    }

  private:
    BufferType& buf_;
    pixfmt_type pixf_;
    renderer_base renb_;
    RasterizerType& ras_;
};

} // namespace detail

template<typename T0, typename T1>
void agg_renderer<T0, T1>::process(markers_symbolizer const& sym,
                                   feature_impl& feature,
                                   proj_transform const& prj_trans)
{
    using color_type = agg::rgba8;
    using order_type = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>; // comp blender
    using buf_type = agg::rendering_buffer;
    using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, buf_type>;
    using renderer_base = agg::renderer_base<pixfmt_comp_type>;
    using renderer_type = agg::renderer_scanline_aa_solid<renderer_base>;
    using svg_renderer_type = svg::renderer_agg<svg_path_adapter, svg_attribute_type, renderer_type, pixfmt_comp_type>;
    ras_ptr->reset();

    double gamma = get<value_double, keys::gamma>(sym, feature, common_.vars_);
    gamma_method_enum gamma_method = get<gamma_method_enum, keys::gamma_method>(sym, feature, common_.vars_);
    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }

    buffer_type& current_buffer = buffers_.top().get();
    buf_type render_buffer(current_buffer.bytes(),
                           current_buffer.width(),
                           current_buffer.height(),
                           current_buffer.row_size());
    box2d<double> clip_box = clipping_extent(common_);

    using renderer_context_type = detail::agg_markers_renderer_context<svg_renderer_type, buf_type, rasterizer>;
    renderer_context_type renderer_context(sym, feature, common_.vars_, render_buffer, *ras_ptr);

    render_markers_symbolizer(sym, feature, prj_trans, common_, clip_box, renderer_context);
}

template void
  agg_renderer<image_rgba8>::process(markers_symbolizer const&, mapnik::feature_impl&, proj_transform const&);
} // namespace mapnik
