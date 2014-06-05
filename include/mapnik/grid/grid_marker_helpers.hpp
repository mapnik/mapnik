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
struct raster_markers_rasterizer_dispatch_grid : mapnik::noncopyable
{
    typedef typename RendererBase::pixfmt_type pixfmt_type;
    typedef typename RendererBase::pixfmt_type::color_type color_type;

    typedef typename std::tuple_element<0,RendererContext>::type BufferType;
    typedef typename std::tuple_element<1,RendererContext>::type RasterizerType;
    typedef typename std::tuple_element<2,RendererContext>::type PixMapType;

    raster_markers_rasterizer_dispatch_grid(image_data_32 const& src,
                                            agg::trans_affine const& marker_trans,
                                            markers_symbolizer const& sym,
                                            Detector & detector,
                                            double scale_factor,
                                            mapnik::feature_impl const& feature,
                                            attributes const& vars,
                                            RendererContext const& renderer_context)
    : buf_(std::get<0>(renderer_context)),
        pixf_(buf_),
        renb_(pixf_),
        ras_(std::get<1>(renderer_context)),
        src_(src),
        marker_trans_(marker_trans),
        sym_(sym),
        detector_(detector),
        scale_factor_(scale_factor),
        feature_(feature),
        vars_(vars),
        pixmap_(std::get<2>(renderer_context)),
        placed_(false) {}

    template <typename T>
    void add_path(T & path)
    {
        agg::scanline_bin sl_;
        marker_placement_enum placement_method = get<marker_placement_enum>(sym_, keys::markers_placement_type, feature_, vars_, MARKER_POINT_PLACEMENT);
        bool ignore_placement = get<bool>(sym_, keys::ignore_placement, feature_, vars_, false);
        bool allow_overlap = get<bool>(sym_, keys::allow_overlap, feature_, vars_, false);

        box2d<double> bbox(0,0, src_.width(),src_.height());
        if (placement_method != MARKER_LINE_PLACEMENT ||
            path.type() == geometry_type::types::Point)
        {
            double x = 0;
            double y = 0;
            if (path.type() == geometry_type::types::LineString)
            {
                if (!label::middle_point(path, x, y))
                {
                    return;
                }
            }
            else if (placement_method == MARKER_INTERIOR_PLACEMENT)
            {
                if (!label::interior_position(path, x, y))
                {
                    return;
                }
            }
            else
            {
                if (!label::centroid(path, x, y))
                {
                    return;
                }
            }
            agg::trans_affine matrix = marker_trans_;
            matrix.translate(x,y);
            box2d<double> transformed_bbox = bbox * matrix;
            if (allow_overlap ||
                detector_.has_placement(transformed_bbox))
            {
                render_raster_marker(matrix);
                if (!ignore_placement)
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
            double spacing = get<double>(sym_, keys::spacing, feature_, vars_, 100.0);
            double max_error = get<double>(sym_, keys::max_error, feature_, vars_, 0.2);
            markers_placement<T, label_collision_detector4> placement(path, bbox, marker_trans_, detector_,
                                                                      spacing * scale_factor_,
                                                                      max_error,
                                                                      allow_overlap);
            double x, y, angle;
            while (placement.get_point(x, y, angle, ignore_placement))
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
    }

private:
    BufferType & buf_;
    pixfmt_type pixf_;
    RendererBase renb_;
    RasterizerType & ras_;
    image_data_32 const& src_;
    agg::trans_affine const& marker_trans_;
    markers_symbolizer const& sym_;
    Detector & detector_;
    double scale_factor_;
    mapnik::feature_impl const& feature_;
    attributes const& vars_;
    PixMapType & pixmap_;
    bool placed_;
};


template <typename SvgRenderer, typename Detector, typename RendererContext>
struct vector_markers_rasterizer_dispatch_grid : mapnik::noncopyable
{
    typedef typename SvgRenderer::renderer_base         renderer_base;
    typedef typename SvgRenderer::vertex_source_type    vertex_source_type;
    typedef typename SvgRenderer::attribute_source_type attribute_source_type;
    typedef typename renderer_base::pixfmt_type         pixfmt_type;

    typedef typename std::tuple_element<0,RendererContext>::type BufferType;
    typedef typename std::tuple_element<1,RendererContext>::type RasterizerType;
    typedef typename std::tuple_element<2,RendererContext>::type PixMapType;

    vector_markers_rasterizer_dispatch_grid(vertex_source_type & path,
                                            attribute_source_type const& attrs,
                                            box2d<double> const& bbox,
                                            agg::trans_affine const& marker_trans,
                                            markers_symbolizer const& sym,
                                            Detector & detector,
                                            double scale_factor,
                                            mapnik::feature_impl const& feature,
                                            attributes const& vars,
                                            bool snap_to_pixels,
                                            RendererContext const& renderer_context)
    : buf_(std::get<0>(renderer_context)),
        pixf_(buf_),
        renb_(pixf_),
        svg_renderer_(path, attrs),
        ras_(std::get<1>(renderer_context)),
        bbox_(bbox),
        marker_trans_(marker_trans),
        sym_(sym),
        detector_(detector),
        scale_factor_(scale_factor),
        feature_(feature),
        vars_(vars),
        pixmap_(std::get<2>(renderer_context)),
        placed_(false)
    {
    }

    template <typename T>
    void add_path(T & path)
    {
        agg::scanline_bin sl_;
        marker_placement_enum placement_method =
            get<marker_placement_enum>(sym_, keys::markers_placement_type, feature_, vars_, MARKER_POINT_PLACEMENT);
        bool ignore_placement = get<bool>(sym_, keys::ignore_placement, feature_, vars_, false);
        double opacity = get<double>(sym_,keys::opacity, feature_, vars_, 1.0);
        bool allow_overlap = get<bool>(sym_, keys::allow_overlap, feature_, vars_, false);

        coord2d center = bbox_.center();
        agg::trans_affine_translation recenter(-center.x, -center.y);

        if (placement_method != MARKER_LINE_PLACEMENT ||
            path.type() == geometry_type::types::Point)
        {
            double x = 0;
            double y = 0;
            if (path.type() == geometry_type::types::LineString)
            {
                if (!label::middle_point(path, x, y)) return;
            }
            else if (placement_method == MARKER_INTERIOR_PLACEMENT)
            {
                if (!label::interior_position(path, x, y)) return;
            }
            else
            {
                if (!label::centroid(path, x, y)) return;
            }

            agg::trans_affine matrix = recenter * marker_trans_;
            matrix.translate(x,y);
            box2d<double> transformed_bbox = bbox_ * matrix;
            if (allow_overlap ||
                detector_.has_placement(transformed_bbox))
            {
                svg_renderer_.render_id(ras_, sl_, renb_, feature_.id(), matrix, opacity, bbox_);
                if (!ignore_placement)
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
            double spacing = get<double>(sym_, keys::spacing, feature_, vars_, 100.0);
            double max_error = get<double>(sym_, keys::max_error, feature_, vars_, 0.2);
            markers_placement<T, Detector> placement(path, bbox_, marker_trans_, detector_,
                                                     spacing * scale_factor_,
                                                     max_error,
                                                     allow_overlap);
            double x, y, angle;
            while (placement.get_point(x, y, angle, ignore_placement))
            {
                agg::trans_affine matrix = recenter * marker_trans_;
                matrix.rotate(angle);
                matrix.translate(x, y);
                svg_renderer_.render_id(ras_, sl_, renb_, feature_.id(), matrix, opacity, bbox_);
                if (!placed_)
                {
                    pixmap_.add_feature(feature_);
                    placed_ = true;
                }
            }
        }
    }
private:
    BufferType & buf_;
    pixfmt_type pixf_;
    renderer_base renb_;
    SvgRenderer svg_renderer_;
    RasterizerType & ras_;
    box2d<double> const& bbox_;
    agg::trans_affine const& marker_trans_;
    markers_symbolizer const& sym_;
    Detector & detector_;
    double scale_factor_;
    mapnik::feature_impl const& feature_;
    attributes const& vars_;
    PixMapType & pixmap_;
    bool placed_;
};

}
#endif
