/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#ifndef MAPNIK_GRID_MARKER_HELPERS_HPP
#define MAPNIK_GRID_MARKER_HELPERS_HPP

// mapnik
#include <mapnik/symbolizer.hpp>
#include <mapnik/markers_placement.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geom_util.hpp>

// agg
#include "agg_renderer_scanline.h"
#include "agg_scanline_bin.h"
#include "agg_image_filters.h"
#include "agg_trans_bilinear.h"
#include "agg_span_allocator.h"
#include "agg_image_accessors.h"
#include "agg_span_image_filter_gray.h"


namespace mapnik {

template <typename RendererBase, typename RendererType, typename Detector, typename RendererContext>
struct raster_markers_rasterizer_dispatch_grid : public raster_markers_dispatch<Detector>
{
    using pixfmt_type = typename RendererBase::pixfmt_type;
    using color_type = typename RendererBase::pixfmt_type::color_type;

    using BufferType = typename std::tuple_element<0,RendererContext>::type;
    using RasterizerType = typename std::tuple_element<1,RendererContext>::type;
    using PixMapType = typename std::tuple_element<2,RendererContext>::type;

    raster_markers_rasterizer_dispatch_grid(image_data_32 const& src,
                                            agg::trans_affine const& marker_trans,
                                            markers_symbolizer const& sym,
                                            Detector & detector,
                                            double scale_factor,
                                            mapnik::feature_impl & feature,
                                            attributes const& vars,
                                            RendererContext const& renderer_context)
    : raster_markers_dispatch<Detector>(src, marker_trans, sym, detector, scale_factor,
                                        feature, vars, false),
        buf_(std::get<0>(renderer_context)),
        pixf_(buf_),
        renb_(pixf_),
        ras_(std::get<1>(renderer_context)),
        pixmap_(std::get<2>(renderer_context)),
        placed_(false)
    {}

    void render_raster_marker(agg::trans_affine const& marker_tr, double opacity)
    {
        agg::scanline_bin sl_;
        double width  = src_.width();
        double height = src_.height();
        double p[8];
        p[0] = 0;     p[1] = 0;
        p[2] = width; p[3] = 0;
        p[4] = width; p[5] = height;
        p[6] = 0;     p[7] = height;
        marker_tr.transform(&p[0], &p[1]);
        marker_tr.transform(&p[2], &p[3]);
        marker_tr.transform(&p[4], &p[5]);
        marker_tr.transform(&p[6], &p[7]);
        ras_.move_to_d(p[0],p[1]);
        ras_.line_to_d(p[2],p[3]);
        ras_.line_to_d(p[4],p[5]);
        ras_.line_to_d(p[6],p[7]);
        RendererType ren(renb_);
        ren.color(color_type(feature_.id()));
        agg::render_scanlines(ras_, sl_, ren);
        if (!placed_)
        {
            pixmap_.add_feature(feature_);
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

    using raster_markers_dispatch<Detector>::src_;
    using raster_markers_dispatch<Detector>::feature_;
    using raster_markers_dispatch<Detector>::snap_to_pixels_;
};


template <typename SvgRenderer, typename Detector, typename RendererContext>
struct vector_markers_rasterizer_dispatch_grid : public vector_markers_dispatch<Detector>
{
    using renderer_base = typename SvgRenderer::renderer_base        ;
    using vertex_source_type = typename SvgRenderer::vertex_source_type   ;
    using attribute_source_type = typename SvgRenderer::attribute_source_type;
    using pixfmt_type = typename renderer_base::pixfmt_type        ;

    using BufferType = typename std::tuple_element<0,RendererContext>::type;
    using RasterizerType = typename std::tuple_element<1,RendererContext>::type;
    using PixMapType = typename std::tuple_element<2,RendererContext>::type;

    vector_markers_rasterizer_dispatch_grid(vertex_source_type & path,
                                            attribute_source_type const& attrs,
                                            box2d<double> const& bbox,
                                            agg::trans_affine const& marker_trans,
                                            markers_symbolizer const& sym,
                                            Detector & detector,
                                            double scale_factor,
                                            mapnik::feature_impl & feature,
                                            attributes const& vars,
                                            bool snap_to_pixels,
                                            RendererContext const& renderer_context)
    : vector_markers_dispatch<Detector>(bbox, marker_trans, sym, detector, scale_factor, feature, vars, snap_to_pixels),
        buf_(std::get<0>(renderer_context)),
        pixf_(buf_),
        renb_(pixf_),
        svg_renderer_(path, attrs),
        ras_(std::get<1>(renderer_context)),
        pixmap_(std::get<2>(renderer_context)),
        placed_(false)
    {}

    void render_vector_marker(agg::trans_affine & marker_tr, double opacity)
    {
        agg::scanline_bin sl_;
        svg_renderer_.render_id(ras_, sl_, renb_, feature_.id(), marker_tr, opacity, bbox_);
        if (!placed_)
        {
            pixmap_.add_feature(feature_);
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

    using vector_markers_dispatch<Detector>::bbox_;
    using vector_markers_dispatch<Detector>::feature_;
    using vector_markers_dispatch<Detector>::snap_to_pixels_;
};

}
#endif
