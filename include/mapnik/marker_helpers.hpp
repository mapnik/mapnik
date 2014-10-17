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

#ifndef MAPNIK_MARKER_HELPERS_HPP
#define MAPNIK_MARKER_HELPERS_HPP

#include <mapnik/color.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/marker.hpp> // for svg_storage_type
#include <mapnik/markers_placement.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/label_collision_detector.hpp>

// agg
#include "agg_ellipse.h"
#include "agg_color_rgba.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_rendering_buffer.h"
#include "agg_scanline_u.h"
#include "agg_image_filters.h"
#include "agg_trans_affine.h"
#include "agg_span_allocator.h"
#include "agg_image_accessors.h"
#include "agg_pixfmt_rgba.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_span_interpolator_linear.h"

// boost
#include <boost/optional.hpp>

// stl
#include <memory>
#include <type_traits> // remove_reference
#include <cmath>

namespace mapnik {

struct clip_poly_tag;

using svg_attribute_type = agg::pod_bvector<svg::path_attributes>;

template <typename SvgRenderer, typename Detector, typename RendererContext>
struct vector_markers_rasterizer_dispatch : mapnik::noncopyable
{
    using renderer_base = typename SvgRenderer::renderer_base        ;
    using vertex_source_type = typename SvgRenderer::vertex_source_type   ;
    using attribute_source_type = typename SvgRenderer::attribute_source_type;
    using pixfmt_type = typename renderer_base::pixfmt_type        ;

    using BufferType = typename std::tuple_element<0,RendererContext>::type;
    using RasterizerType = typename std::tuple_element<1,RendererContext>::type;

    vector_markers_rasterizer_dispatch(vertex_source_type & path,
                                       attribute_source_type const& attrs,
                                       box2d<double> const& bbox,
                                       agg::trans_affine const& marker_trans,
                                       symbolizer_base const& sym,
                                       Detector & detector,
                                       double scale_factor,
                                       feature_impl & feature,
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
        feature_(feature),
        vars_(vars),
        scale_factor_(scale_factor),
        snap_to_pixels_(snap_to_pixels)
    {
        pixf_.comp_op(static_cast<agg::comp_op_e>(get<composite_mode_e, keys::comp_op>(sym,feature_, vars_)));
    }

    template <typename T>
    void add_path(T & path)
    {
        agg::scanline_u8 sl_;
        marker_placement_enum placement_method = get<marker_placement_enum, keys::markers_placement_type>(sym_, feature_, vars_);
        value_bool ignore_placement = get<value_bool, keys::ignore_placement>(sym_, feature_, vars_);
        value_bool allow_overlap = get<value_bool, keys::allow_overlap>(sym_, feature_, vars_);
        value_bool avoid_edges = get<value_bool, keys::avoid_edges>(sym_, feature_, vars_);
        value_double opacity = get<value_double,keys::opacity>(sym_, feature_, vars_);
        value_double spacing = get<value_double, keys::spacing>(sym_, feature_, vars_);
        value_double max_error = get<value_double, keys::max_error>(sym_, feature_, vars_);
        coord2d center = bbox_.center();
        agg::trans_affine_translation recenter(-center.x, -center.y);
        agg::trans_affine tr = recenter * marker_trans_;
        markers_placement_params params { bbox_, tr, spacing * scale_factor_, max_error, allow_overlap, avoid_edges };
        markers_placement_finder<T, Detector> placement_finder(
            placement_method, path, detector_, params);
        double x, y, angle = .0;
        while (placement_finder.get_point(x, y, angle, ignore_placement))
        {
            agg::trans_affine matrix = tr;
            matrix.rotate(angle);
            matrix.translate(x, y);
            if (snap_to_pixels_)
            {
                // https://github.com/mapnik/mapnik/issues/1316
                matrix.tx = std::floor(matrix.tx + .5);
                matrix.ty = std::floor(matrix.ty + .5);
            }
            svg_renderer_.render(ras_, sl_, renb_, matrix, opacity, bbox_);
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
    symbolizer_base const& sym_;
    Detector & detector_;
    feature_impl & feature_;
    attributes const& vars_;
    double scale_factor_;
    bool snap_to_pixels_;
};

template <typename Detector,typename RendererContext>
struct raster_markers_rasterizer_dispatch : mapnik::noncopyable
{
    using BufferType = typename std::remove_reference<typename std::tuple_element<0,RendererContext>::type>::type;
    using RasterizerType = typename std::tuple_element<1,RendererContext>::type;

    using color_type = agg::rgba8;
    using order_type = agg::order_rgba;
    using pixel_type = agg::pixel32_type;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>; // comp blender
    using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, BufferType>;
    using renderer_base = agg::renderer_base<pixfmt_comp_type>;

    raster_markers_rasterizer_dispatch(image_data_32 const& src,
                                       agg::trans_affine const& marker_trans,
                                       symbolizer_base const& sym,
                                       Detector & detector,
                                       double scale_factor,
                                       feature_impl & feature,
                                       attributes const& vars,
                                       RendererContext const& renderer_context,
                                       bool snap_to_pixels = false)
    : buf_(std::get<0>(renderer_context)),
        pixf_(buf_),
        renb_(pixf_),
        ras_(std::get<1>(renderer_context)),
        src_(src),
        marker_trans_(marker_trans),
        sym_(sym),
        detector_(detector),
        feature_(feature),
        vars_(vars),
        scale_factor_(scale_factor),
        snap_to_pixels_(snap_to_pixels)
    {
        pixf_.comp_op(static_cast<agg::comp_op_e>(get<composite_mode_e, keys::comp_op>(sym, feature_, vars_)));
    }

    template <typename T>
    void add_path(T & path)
    {
        marker_placement_enum placement_method = get<marker_placement_enum, keys::markers_placement_type>(sym_, feature_, vars_);
        value_bool allow_overlap = get<value_bool, keys::allow_overlap>(sym_, feature_, vars_);
        value_bool avoid_edges = get<value_bool, keys::avoid_edges>(sym_, feature_, vars_);
        box2d<double> bbox_(0,0, src_.width(),src_.height());
        value_double opacity = get<value_double, keys::opacity>(sym_, feature_, vars_);
        value_bool ignore_placement = get<value_bool, keys::ignore_placement>(sym_, feature_, vars_);
        value_double spacing = get<value_double, keys::spacing>(sym_, feature_, vars_);
        value_double max_error = get<value_double, keys::max_error>(sym_, feature_, vars_);
        markers_placement_params params { bbox_, marker_trans_, spacing * scale_factor_, max_error, allow_overlap, avoid_edges };
        markers_placement_finder<T, label_collision_detector4> placement_finder(
            placement_method, path, detector_, params);
        double x, y, angle = .0;
        while (placement_finder.get_point(x, y, angle, ignore_placement))
        {
            agg::trans_affine matrix = marker_trans_;
            matrix.rotate(angle);
            matrix.translate(x, y);
            render_raster_marker(matrix, opacity);
        }
    }

    void render_raster_marker(agg::trans_affine const& marker_tr,
                              double opacity)
    {
        using pixfmt_pre = agg::pixfmt_rgba32_pre;
        agg::scanline_u8 sl_;
        double width  = src_.width();
        double height = src_.height();
        if (std::fabs(1.0 - scale_factor_) < 0.001
            && (std::fabs(1.0 - marker_tr.sx) < agg::affine_epsilon)
            && (std::fabs(0.0 - marker_tr.shy) < agg::affine_epsilon)
            && (std::fabs(0.0 - marker_tr.shx) < agg::affine_epsilon)
            && (std::fabs(1.0 - marker_tr.sy) < agg::affine_epsilon))
        {
            agg::rendering_buffer src_buffer((unsigned char *)src_.getBytes(),src_.width(),src_.height(),src_.width() * 4);
            pixfmt_pre pixf_mask(src_buffer);
            if (snap_to_pixels_)
            {
                renb_.blend_from(pixf_mask,
                                 0,
                                 std::floor(marker_tr.tx + .5),
                                 std::floor(marker_tr.ty + .5),
                                 unsigned(255*opacity));
            }
            else
            {
                renb_.blend_from(pixf_mask,
                                 0,
                                 marker_tr.tx,
                                 marker_tr.ty,
                                 unsigned(255*opacity));
            }
        }
        else
        {
            using img_accessor_type = agg::image_accessor_clone<pixfmt_pre>;
            using interpolator_type = agg::span_interpolator_linear<>;
            //using span_gen_type = agg::span_image_filter_rgba_2x2<img_accessor_type,interpolator_type>;
            using span_gen_type = agg::span_image_resample_rgba_affine<img_accessor_type>;
            using renderer_type = agg::renderer_scanline_aa_alpha<renderer_base,
                                                                  agg::span_allocator<color_type>,
                                                                  span_gen_type>;

            double p[8];
            p[0] = 0;     p[1] = 0;
            p[2] = width; p[3] = 0;
            p[4] = width; p[5] = height;
            p[6] = 0;     p[7] = height;
            marker_tr.transform(&p[0], &p[1]);
            marker_tr.transform(&p[2], &p[3]);
            marker_tr.transform(&p[4], &p[5]);
            marker_tr.transform(&p[6], &p[7]);
            agg::span_allocator<color_type> sa;
            agg::image_filter_lut filter;
            filter.calculate(agg::image_filter_bilinear(), true);
            agg::rendering_buffer marker_buf((unsigned char *)src_.getBytes(),
                                             src_.width(),
                                             src_.height(),
                                             src_.width()*4);
            pixfmt_pre pixf(marker_buf);
            img_accessor_type ia(pixf);
            agg::trans_affine final_tr(p, 0, 0, width, height);
            if (snap_to_pixels_)
            {
                final_tr.tx = std::floor(final_tr.tx+.5);
                final_tr.ty = std::floor(final_tr.ty+.5);
            }
            interpolator_type interpolator(final_tr);
            span_gen_type sg(ia, interpolator, filter);
            renderer_type rp(renb_,sa, sg, unsigned(opacity*255));
            ras_.move_to_d(p[0],p[1]);
            ras_.line_to_d(p[2],p[3]);
            ras_.line_to_d(p[4],p[5]);
            ras_.line_to_d(p[6],p[7]);
            agg::render_scanlines(ras_, sl_, rp);
        }
    }

private:
    BufferType & buf_;
    pixfmt_comp_type pixf_;
    renderer_base renb_;
    RasterizerType & ras_;
    image_data_32 const& src_;
    agg::trans_affine const& marker_trans_;
    symbolizer_base const& sym_;
    Detector & detector_;
    feature_impl & feature_;
    attributes const& vars_;
    double scale_factor_;
    bool snap_to_pixels_;
};


void build_ellipse(symbolizer_base const& sym, mapnik::feature_impl & feature, attributes const& vars, svg_storage_type & marker_ellipse, svg::svg_path_adapter & svg_path);

bool push_explicit_style(svg_attribute_type const& src,
                         svg_attribute_type & dst,
                         symbolizer_base const& sym,
                         feature_impl & feature,
                         attributes const& vars);

void setup_transform_scaling(agg::trans_affine & tr,
                             double svg_width,
                             double svg_height,
                             mapnik::feature_impl & feature,
                             attributes const& vars,
                             symbolizer_base const& sym);

// Apply markers to a feature with multiple geometries
template <typename Converter>
void apply_markers_multi(feature_impl const& feature, attributes const& vars, Converter & converter, symbolizer_base const& sym)
{
    std::size_t geom_count = feature.paths().size();
    if (geom_count == 1)
    {
        converter.apply(const_cast<geometry_type&>(feature.paths()[0]));
    }
    else if (geom_count > 1)
    {
        marker_multi_policy_enum multi_policy = get<marker_multi_policy_enum, keys::markers_multipolicy>(sym, feature, vars);
        marker_placement_enum placement = get<marker_placement_enum, keys::markers_placement_type>(sym, feature, vars);

        if (placement == MARKER_POINT_PLACEMENT &&
            multi_policy == MARKER_WHOLE_MULTI)
        {
            double x, y;
            if (label::centroid_geoms(feature.paths().begin(), feature.paths().end(), x, y))
            {
                geometry_type pt(geometry_type::types::Point);
                pt.move_to(x, y);
                // unset any clipping since we're now dealing with a point
                converter.template unset<clip_poly_tag>();
                converter.apply(pt);
            }
        }
        else if ((placement == MARKER_POINT_PLACEMENT || placement == MARKER_INTERIOR_PLACEMENT) &&
                 multi_policy == MARKER_LARGEST_MULTI)
        {
            // Only apply to path with largest envelope area
            // TODO: consider using true area for polygon types
            double maxarea = 0;
            geometry_type const* largest = 0;
            for (geometry_type const& geom : feature.paths())
            {
                const box2d<double>& env = geom.envelope();
                double area = env.width() * env.height();
                if (area > maxarea)
                {
                    maxarea = area;
                    largest = &geom;
                }
            }
            if (largest)
            {
                converter.apply(const_cast<geometry_type&>(*largest));
            }
        }
        else
        {
            if (multi_policy != MARKER_EACH_MULTI && placement != MARKER_POINT_PLACEMENT)
            {
                MAPNIK_LOG_WARN(marker_symbolizer) << "marker_multi_policy != 'each' has no effect with marker_placement != 'point'";
            }
            for (geometry_type const& path : feature.paths())
            {
                converter.apply(const_cast<geometry_type&>(path));
            }
        }
    }
}

}

#endif //MAPNIK_MARKER_HELPERS_HPP
