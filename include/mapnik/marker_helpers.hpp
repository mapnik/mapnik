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

namespace mapnik {

struct clip_poly_tag;

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
                                       markers_symbolizer const& sym,
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
        pixf_.comp_op(static_cast<agg::comp_op_e>(get<composite_mode_e>(sym, keys::comp_op, feature_, vars_, src_over)));
    }

    template <typename T>
    void add_path(T & path)
    {
        agg::scanline_u8 sl_;
        marker_placement_enum placement_method = get<marker_placement_enum>(sym_, keys::markers_placement_type, feature_, vars_, MARKER_POINT_PLACEMENT);
        bool ignore_placement = get<bool>(sym_, keys::ignore_placement, feature_, vars_, false);
        bool allow_overlap = get<bool>(sym_, keys::allow_overlap, feature_, vars_, false);
        bool avoid_edges = get<bool>(sym_, keys::avoid_edges, feature_, vars_, false);
        double opacity = get<double>(sym_,keys::opacity, feature_, vars_, 1.0);
        double spacing = get<double>(sym_, keys::spacing, feature_, vars_, 100.0);
        double max_error = get<double>(sym_, keys::max_error, feature_, vars_, 0.2);
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
    markers_symbolizer const& sym_;
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
                                       markers_symbolizer const& sym,
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
        pixf_.comp_op(static_cast<agg::comp_op_e>(get<composite_mode_e>(sym, keys::comp_op, feature_, vars_, src_over)));
    }

    template <typename T>
    void add_path(T & path)
    {
        marker_placement_enum placement_method = get<marker_placement_enum>(sym_, keys::markers_placement_type, feature_, vars_, MARKER_POINT_PLACEMENT);
        bool allow_overlap = get<bool>(sym_, keys::allow_overlap, feature_, vars_, false);
        bool avoid_edges = get<bool>(sym_, keys::avoid_edges, feature_, vars_, false);
        box2d<double> bbox_(0,0, src_.width(),src_.height());
        double opacity = get<double>(sym_, keys::opacity, feature_, vars_, 1.0);
        bool ignore_placement = get<bool>(sym_, keys::ignore_placement, feature_, vars_, false);
        double spacing = get<double>(sym_, keys::spacing, feature_, vars_, 100.0);
        double max_error = get<double>(sym_, keys::max_error, feature_, vars_, 0.2);
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
    markers_symbolizer const& sym_;
    Detector & detector_;
    feature_impl & feature_;
    attributes const& vars_;
    double scale_factor_;
    bool snap_to_pixels_;
};


template <typename T>
void build_ellipse(T const& sym, mapnik::feature_impl & feature, attributes const& vars, svg_storage_type & marker_ellipse, svg::svg_path_adapter & svg_path)
{
    auto width_expr  = get<expression_ptr>(sym, keys::width);
    auto height_expr = get<expression_ptr>(sym, keys::height);
    double width = 0;
    double height = 0;
    if (width_expr && height_expr)
    {
        width = util::apply_visitor(evaluate<feature_impl,value_type,attributes>(feature,vars), *width_expr).to_double();
        height = util::apply_visitor(evaluate<feature_impl,value_type,attributes>(feature,vars), *height_expr).to_double();
    }
    else if (width_expr)
    {
        width = util::apply_visitor(evaluate<feature_impl,value_type,attributes>(feature,vars), *width_expr).to_double();
        height = width;
    }
    else if (height_expr)
    {
        height = util::apply_visitor(evaluate<feature_impl,value_type,attributes>(feature,vars), *height_expr).to_double();
        width = height;
    }
    svg::svg_converter_type styled_svg(svg_path, marker_ellipse.attributes());
    styled_svg.push_attr();
    styled_svg.begin_path();
    agg::ellipse c(0, 0, width/2.0, height/2.0);
    styled_svg.storage().concat_path(c);
    styled_svg.end_path();
    styled_svg.pop_attr();
    double lox,loy,hix,hiy;
    styled_svg.bounding_rect(&lox, &loy, &hix, &hiy);
    styled_svg.set_dimensions(width,height);
    marker_ellipse.set_dimensions(width,height);
    marker_ellipse.set_bounding_box(lox,loy,hix,hiy);
}

template <typename Attr>
bool push_explicit_style(Attr const& src, Attr & dst,
                         markers_symbolizer const& sym,
                         feature_impl & feature,
                         attributes const& vars)
{
    auto fill_color = get_optional<color>(sym, keys::fill, feature, vars);
    auto fill_opacity = get_optional<double>(sym, keys::fill_opacity, feature, vars);
    auto stroke_color = get_optional<color>(sym, keys::stroke, feature, vars);
    auto stroke_width = get_optional<double>(sym, keys::stroke_width, feature, vars);
    auto stroke_opacity = get_optional<double>(sym, keys::stroke_opacity, feature, vars);
    if (fill_color ||
        fill_opacity ||
        stroke_color ||
        stroke_width ||
        stroke_opacity)
    {
        bool success = false;
        for(unsigned i = 0; i < src.size(); ++i)
        {
            success = true;
            dst.push_back(src[i]);
            mapnik::svg::path_attributes & attr = dst.last();
            if (attr.stroke_flag)
            {
                if (stroke_width)
                {
                    attr.stroke_width = *stroke_width;
                }
                if (stroke_color)
                {
                    color const& s_color = *stroke_color;
                    attr.stroke_color = agg::rgba(s_color.red()/255.0,
                                                  s_color.green()/255.0,
                                                  s_color.blue()/255.0,
                                                  s_color.alpha()/255.0);
                }
                if (stroke_opacity)
                {
                    attr.stroke_opacity = *stroke_opacity;
                }
            }
            if (attr.fill_flag)
            {
                if (fill_color)
                {
                    color const& f_color = *fill_color;
                    attr.fill_color = agg::rgba(f_color.red()/255.0,
                                                f_color.green()/255.0,
                                                f_color.blue()/255.0,
                                                f_color.alpha()/255.0);
                }
                if (fill_opacity)
                {
                    attr.fill_opacity = *fill_opacity;
                }
            }
        }
        return success;
    }
    return false;
}

template <typename T>
void setup_transform_scaling(agg::trans_affine & tr,
                             double svg_width,
                             double svg_height,
                             mapnik::feature_impl & feature,
                             attributes const& vars,
                             T const& sym)
{
    double width = get<double>(sym, keys::width, feature, vars, 0);
    double height = get<double>(sym, keys::height, feature, vars, 0);

    if (width > 0 && height > 0)
    {
        double sx = width/svg_width;
        double sy = height/svg_height;
        tr *= agg::trans_affine_scaling(sx,sy);
    }
    else if (width > 0)
    {
        double sx = width/svg_width;
        tr *= agg::trans_affine_scaling(sx);
    }
    else if (height > 0)
    {
        double sy = height/svg_height;
        tr *= agg::trans_affine_scaling(sy);
    }
}

// Apply markers to a feature with multiple geometries
template <typename Converter>
void apply_markers_multi(feature_impl const& feature, attributes const& vars, Converter & converter, markers_symbolizer const& sym)
{
    std::size_t geom_count = feature.paths().size();
    if (geom_count == 1)
    {
        converter.apply(feature.paths()[0]);
    }
    else if (geom_count > 1)
    {
        marker_multi_policy_enum multi_policy = get<marker_multi_policy_enum>(sym, keys::markers_multipolicy, feature, vars, MARKER_EACH_MULTI);
        marker_placement_enum placement = get<marker_placement_enum>(sym, keys::markers_placement_type, feature, vars, MARKER_POINT_PLACEMENT);

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
                converter.apply(*largest);
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
                converter.apply(path);
            }
        }
    }
}

}

#endif //MAPNIK_MARKER_HELPERS_HPP
