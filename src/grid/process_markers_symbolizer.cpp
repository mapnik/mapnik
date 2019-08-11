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

/*

porting notes -->

 - rasterizer -> grid_rasterizer
 - current_buffer_ -> pixmap_
 - agg::rendering_buffer -> grid_renderering_buffer
 - no gamma
 - agg::scanline_bin sl
 - grid_rendering_buffer
 - agg::renderer_scanline_bin_solid
 - TODO - clamp sizes to > 4 pixels of interactivity
 - svg_renderer.render_id
 - only encode feature if placements are found:
    if (placed)
    {
        pixmap_.add_feature(feature);
    }

*/

// mapnik
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid_render_marker.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/renderer_common/render_markers_symbolizer.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#pragma GCC diagnostic pop

namespace mapnik {

namespace detail {

template <typename SvgRenderer, typename ScanlineRenderer,
          typename BufferType, typename RasterizerType, typename PixMapType>
struct grid_markers_renderer_context : markers_renderer_context
{
    using renderer_base = typename SvgRenderer::renderer_base;
    using vertex_source_type = typename SvgRenderer::vertex_source_type;
    using attribute_source_type = typename SvgRenderer::attribute_source_type;
    using pixfmt_type = typename renderer_base::pixfmt_type;

    grid_markers_renderer_context(feature_impl const& feature,
                                  BufferType & buf,
                                  RasterizerType & ras,
                                  PixMapType & pixmap)
      : feature_(feature),
        buf_(buf),
        pixf_(buf_),
        renb_(pixf_),
        ras_(ras),
        pixmap_(pixmap),
        placed_(false)
    {}

    virtual void render_marker(svg_path_ptr const& src,
                               svg_path_adapter & path,
                               svg_attribute_type const& attrs,
                               markers_dispatch_params const& params,
                               agg::trans_affine const& marker_tr)
    {
        SvgRenderer svg_renderer_(path, attrs);
        agg::scanline_bin sl_;
        svg_renderer_.render_id(ras_, sl_, renb_, feature_.id(), marker_tr,
                                params.opacity, src->bounding_box());
        place_feature();
    }

    virtual void render_marker(image_rgba8 const& src,
                               markers_dispatch_params const& params,
                               agg::trans_affine const& marker_tr)
    {
        // In the long term this should be a visitor pattern based on the type of
        // render src provided that converts the destination pixel type required.
        render_raster_marker(ScanlineRenderer(renb_), ras_, src, feature_,
                             marker_tr, params.opacity);
        place_feature();
    }

    void place_feature()
    {
        if (!placed_)
        {
            pixmap_.add_feature(feature_);
            placed_ = true;
        }
    }

private:
    feature_impl const& feature_;
    BufferType & buf_;
    pixfmt_type pixf_;
    renderer_base renb_;
    RasterizerType & ras_;
    PixMapType & pixmap_;
    bool placed_;
};

} // namespace detail

template <typename T>
void grid_renderer<T>::process(markers_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    using buf_type = grid_rendering_buffer;
    using pixfmt_type = typename grid_renderer_base_type::pixfmt_type;
    using renderer_type = agg::renderer_scanline_bin_solid<grid_renderer_base_type>;
    using svg_renderer_type = svg::renderer_agg<svg_path_adapter,
                                                svg_attribute_type,
                                                renderer_type,
                                                pixfmt_type>;

    buf_type render_buf(pixmap_.raw_data(), common_.width_, common_.height_, common_.width_);
    ras_ptr->reset();
    box2d<double> clip_box = common_.query_extent_;

    using renderer_context_type = detail::grid_markers_renderer_context<svg_renderer_type,
                                                               renderer_type,
                                                               buf_type,
                                                               grid_rasterizer,
                                                               buffer_type>;
    renderer_context_type renderer_context(feature, render_buf, *ras_ptr, pixmap_);

    render_markers_symbolizer(
        sym, feature, prj_trans, common_, clip_box, renderer_context);
}

template void grid_renderer<grid>::process(markers_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);
} // namespace mapnik

#endif
