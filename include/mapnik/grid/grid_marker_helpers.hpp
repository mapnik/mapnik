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
#include <mapnik/markers_symbolizer.hpp>
#include <mapnik/markers_placement.hpp>
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

template <typename BufferType, typename Rasterizer, typename PixFmt, typename RendererBase, typename RendererType, typename Detector, typename PixMapType>
struct raster_markers_rasterizer_dispatch_grid
{
    typedef mapnik::gray32 color_type;
    typedef typename RendererBase::pixfmt_type pixfmt_type;

    raster_markers_rasterizer_dispatch_grid(BufferType & render_buffer,
                                       Rasterizer & ras,
                                       image_data_32 const& src,
                                       agg::trans_affine const& marker_trans,
                                       markers_symbolizer const& sym,
                                       Detector & detector,
                                       double scale_factor,
                                       mapnik::feature_impl & feature,
                                       PixMapType & pixmap)
        : buf_(render_buffer),
        pixf_(buf_),
        renb_(pixf_),
        ras_(ras),
        src_(src),
        marker_trans_(marker_trans),
        sym_(sym),
        detector_(detector),
        scale_factor_(scale_factor),
        feature_(feature),
        pixmap_(pixmap),
        placed_(false)
    {
        // TODO - support basic binary operators
        //pixf_.comp_op(static_cast<agg::comp_op_e>(sym_.comp_op()));
    }

    template <typename T>
    void add_path(T & path)
    {
        marker_placement_e placement_method = sym_.get_marker_placement();
        box2d<double> bbox_(0,0, src_.width(),src_.height());
        if (placement_method != MARKER_LINE_PLACEMENT)
        {
            double x = 0;
            double y = 0;
            if (placement_method == MARKER_INTERIOR_PLACEMENT)
            {
                if (!label::interior_position(path, x, y))
                    return;
            }
            else
            {
                if (!label::centroid(path, x, y))
                    return;
            }
            agg::trans_affine matrix = marker_trans_;
            matrix.translate(x,y);
            box2d<double> transformed_bbox = bbox_ * matrix;
            if (sym_.get_allow_overlap() ||
                detector_.has_placement(transformed_bbox))
            {
                render_raster_marker(matrix);
                if (!sym_.get_ignore_placement())
                {
                    detector_.insert(transformed_bbox);
                }
                if (!placed_)
                {
                    pixmap_.add_feature(feature_);
                    placed_ = true;
                }
            }
        }
        else
        {
            markers_placement<T, label_collision_detector4> placement(path, bbox_, marker_trans_, detector_,
                                                                      sym_.get_spacing() * scale_factor_,
                                                                      sym_.get_max_error(),
                                                                      sym_.get_allow_overlap());
            double x, y, angle;
            while (placement.get_point(x, y, angle))
            {
                agg::trans_affine matrix = marker_trans_;
                matrix.rotate(angle);
                matrix.translate(x, y);
                render_raster_marker(matrix);
                if (!placed_)
                {
                    pixmap_.add_feature(feature_);
                    placed_ = true;
                }
            }
        }
    }

    void render_raster_marker(agg::trans_affine const& marker_tr)
    {
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
        ren.color(mapnik::gray32(feature_.id()));
        agg::render_scanlines(ras_, sl_, ren);
    }

private:
    agg::scanline_bin sl_;
    BufferType & buf_;
    PixFmt pixf_;
    RendererBase renb_;
    Rasterizer & ras_;
    image_data_32 const& src_;
    agg::trans_affine const& marker_trans_;
    markers_symbolizer const& sym_;
    Detector & detector_;
    double scale_factor_;
    mapnik::feature_impl & feature_;
    PixMapType & pixmap_;
    bool placed_;
};


template <typename BufferType, typename SvgRenderer, typename Rasterizer, typename Detector, typename PixMapType>
struct vector_markers_rasterizer_dispatch_grid
{
    typedef typename SvgRenderer::renderer_base renderer_base;
    typedef typename renderer_base::pixfmt_type pixfmt_type;

    vector_markers_rasterizer_dispatch_grid(BufferType & render_buffer,
                                SvgRenderer & svg_renderer,
                                Rasterizer & ras,
                                box2d<double> const& bbox,
                                agg::trans_affine const& marker_trans,
                                markers_symbolizer const& sym,
                                Detector & detector,
                                double scale_factor,
                                mapnik::feature_impl & feature,
                                PixMapType & pixmap)
        : buf_(render_buffer),
        pixf_(buf_),
        renb_(pixf_),
        svg_renderer_(svg_renderer),
        ras_(ras),
        bbox_(bbox),
        marker_trans_(marker_trans),
        sym_(sym),
        detector_(detector),
        scale_factor_(scale_factor),
        feature_(feature),
        pixmap_(pixmap),
        placed_(false)
    {
        // TODO
        //pixf_.comp_op(static_cast<agg::comp_op_e>(sym_.comp_op()));
    }

    template <typename T>
    void add_path(T & path)
    {
        marker_placement_e placement_method = sym_.get_marker_placement();
        if (placement_method != MARKER_LINE_PLACEMENT)
        {
            double x = 0;
            double y = 0;
            if (placement_method == MARKER_INTERIOR_PLACEMENT)
            {
                if (!label::interior_position(path, x, y))
                    return;
            }
            else
            {
                if (!label::centroid(path, x, y))
                    return;
            }
            agg::trans_affine matrix = marker_trans_;
            matrix.translate(x,y);
            box2d<double> transformed_bbox = bbox_ * matrix;
            if (sym_.get_allow_overlap() ||
                detector_.has_placement(transformed_bbox))
            {
                svg_renderer_.render_id(ras_, sl_, renb_, feature_.id(), matrix, sym_.get_opacity(), bbox_);
                if (!sym_.get_ignore_placement())
                {
                    detector_.insert(transformed_bbox);
                }
                if (!placed_)
                {
                    pixmap_.add_feature(feature_);
                    placed_ = true;
                }
            }
        }
        else
        {
            markers_placement<T, Detector> placement(path, bbox_, marker_trans_, detector_,
                                                     sym_.get_spacing() * scale_factor_,
                                                     sym_.get_max_error(),
                                                     sym_.get_allow_overlap());
            double x, y, angle;
            while (placement.get_point(x, y, angle))
            {
                agg::trans_affine matrix = marker_trans_;
                matrix.rotate(angle);
                matrix.translate(x, y);
                svg_renderer_.render_id(ras_, sl_, renb_, feature_.id(), matrix, sym_.get_opacity(), bbox_);
                if (!placed_)
                {
                    pixmap_.add_feature(feature_);
                    placed_ = true;
                }
            }
        }
    }
private:
    agg::scanline_bin sl_;
    BufferType & buf_;
    pixfmt_type pixf_;
    renderer_base renb_;
    SvgRenderer & svg_renderer_;
    Rasterizer & ras_;
    box2d<double> const& bbox_;
    agg::trans_affine const& marker_trans_;
    markers_symbolizer const& sym_;
    Detector & detector_;
    double scale_factor_;
    mapnik::feature_impl & feature_;
    PixMapType & pixmap_;
    bool placed_;
};


}
#endif

