/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/feature.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid_render_marker.hpp>
#include <mapnik/grid/grid.hpp>

#include <mapnik/debug.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/renderer_common/process_markers_symbolizer.hpp>

// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"

// boost
#include <boost/optional.hpp>

// stl
#include <algorithm>
#include <tuple>

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
    using PixMapType = typename std::tuple_element<2,RendererContext>::type;

    vector_markers_rasterizer_dispatch(svg_path_ptr const& src,
                                       vertex_source_type & path,
                                       svg_attribute_type const& attrs,
                                       agg::trans_affine const& marker_trans,
                                       markers_symbolizer const& sym,
                                       Detector & detector,
                                       double scale_factor,
                                       mapnik::feature_impl & feature,
                                       attributes const& vars,
                                       bool snap_to_pixels,
                                       RendererContext const& renderer_context)
    : vector_markers_dispatch<Detector>(src, marker_trans, sym, detector, scale_factor, feature, vars),
        buf_(std::get<0>(renderer_context)),
        pixf_(buf_),
        renb_(pixf_),
        svg_renderer_(path, attrs),
        ras_(std::get<1>(renderer_context)),
        pixmap_(std::get<2>(renderer_context)),
        placed_(false)
    {}

    void render_marker(agg::trans_affine const& marker_tr, double opacity)
    {
        agg::scanline_bin sl_;
        svg_renderer_.render_id(ras_, sl_, renb_, this->feature_.id(), marker_tr, opacity, this->src_->bounding_box());
        if (!placed_)
        {
            pixmap_.add_feature(this->feature_);
            placed_ = true;
        }
    }

private:
    BufferType & buf_;
    pixfmt_type pixf_;
    renderer_base renb_;
    SvgRenderer svg_renderer_;
    RasterizerType & ras_;
    PixMapType & pixmap_;
    bool placed_;
};

template <typename RendererBase, typename RendererType, typename Detector, typename RendererContext>
struct raster_markers_rasterizer_dispatch : public raster_markers_dispatch<Detector>
{
    using pixfmt_type = typename RendererBase::pixfmt_type;
    using color_type = typename RendererBase::pixfmt_type::color_type;

    using BufferType = typename std::tuple_element<0,RendererContext>::type;
    using RasterizerType = typename std::tuple_element<1,RendererContext>::type;
    using PixMapType = typename std::tuple_element<2,RendererContext>::type;

    raster_markers_rasterizer_dispatch(image_rgba8 const& src,
                                       agg::trans_affine const& marker_trans,
                                       markers_symbolizer const& sym,
                                       Detector & detector,
                                       double scale_factor,
                                       mapnik::feature_impl & feature,
                                       attributes const& vars,
                                       RendererContext const& renderer_context)
    : raster_markers_dispatch<Detector>(src, marker_trans, sym, detector, scale_factor, feature, vars),
        buf_(std::get<0>(renderer_context)),
        pixf_(buf_),
        renb_(pixf_),
        ras_(std::get<1>(renderer_context)),
        pixmap_(std::get<2>(renderer_context)),
        placed_(false)
    {}

    void render_marker(agg::trans_affine const& marker_tr, double opacity)
    {
        // In the long term this should be a visitor pattern based on the type of render this->src_ provided that converts
        // the destination pixel type required.
        render_raster_marker(RendererType(renb_), ras_, this->src_, this->feature_, marker_tr, opacity);
        if (!placed_)
        {
            pixmap_.add_feature(this->feature_);
            placed_ = true;
        }
    }

private:
    BufferType & buf_;
    pixfmt_type pixf_;
    RendererBase renb_;
    RasterizerType & ras_;
    PixMapType & pixmap_;
    bool placed_;
};

}

template <typename T>
void grid_renderer<T>::process(markers_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    using buf_type = grid_rendering_buffer;
    using pixfmt_type = typename grid_renderer_base_type::pixfmt_type;
    using renderer_type = agg::renderer_scanline_bin_solid<grid_renderer_base_type>;
    using detector_type = label_collision_detector4;

    using namespace mapnik::svg;
    using svg_attribute_type = agg::pod_bvector<path_attributes>;
    using svg_renderer_type = svg_renderer_agg<svg_path_adapter,
                                               svg_attribute_type,
                                               renderer_type,
                                               pixfmt_type>;

    buf_type render_buf(pixmap_.raw_data(), common_.width_, common_.height_, common_.width_);
    ras_ptr->reset();
    box2d<double> clip_box = common_.query_extent_;

    auto renderer_context = std::tie(render_buf,*ras_ptr,pixmap_);
    using context_type = decltype(renderer_context);
    using vector_dispatch_type = detail::vector_markers_rasterizer_dispatch<svg_renderer_type,
                                                                            detector_type,
                                                                            context_type>;
    using raster_dispatch_type = detail::raster_markers_rasterizer_dispatch<grid_renderer_base_type,
                                                                            renderer_type,
                                                                            detector_type,
                                                                            context_type>;
    render_markers_symbolizer<vector_dispatch_type, raster_dispatch_type>(
        sym, feature, prj_trans, common_, clip_box,renderer_context);
}

template void grid_renderer<grid>::process(markers_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);
}

#endif
