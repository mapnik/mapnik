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
#include <mapnik/geometry.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/markers_symbolizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/marker.hpp> // for svg_storage_type
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/markers_placement.hpp>

// agg
#include "agg_ellipse.h"
#include "agg_basics.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_rendering_buffer.h"
#include "agg_scanline_u.h"
#include "agg_image_filters.h"
#include "agg_trans_bilinear.h"
#include "agg_span_allocator.h"
#include "agg_image_accessors.h"
#include "agg_pixfmt_rgba.h"
#include "agg_span_image_filter_rgba.h"

// boost
#include <boost/optional.hpp>

namespace mapnik {


template <typename BufferType, typename SvgRenderer, typename Rasterizer, typename Detector>
struct vector_markers_rasterizer_dispatch
{
    typedef typename SvgRenderer::renderer_base renderer_base;
    typedef typename renderer_base::pixfmt_type pixfmt_type;

    vector_markers_rasterizer_dispatch(BufferType & render_buffer,
                                SvgRenderer & svg_renderer,
                                Rasterizer & ras,
                                box2d<double> const& bbox,
                                agg::trans_affine const& marker_trans,
                                markers_symbolizer const& sym,
                                Detector & detector,
                                double scale_factor)
        : buf_(render_buffer),
        pixf_(buf_),
        renb_(pixf_),
        svg_renderer_(svg_renderer),
        ras_(ras),
        bbox_(bbox),
        marker_trans_(marker_trans),
        sym_(sym),
        detector_(detector),
        scale_factor_(scale_factor)
    {
        pixf_.comp_op(static_cast<agg::comp_op_e>(sym_.comp_op()));
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
                svg_renderer_.render(ras_, sl_, renb_, matrix, sym_.get_opacity(), bbox_);
                if (!sym_.get_ignore_placement())
                {
                    detector_.insert(transformed_bbox);
                }
            }
        }
        else
        {
            markers_placement<T, Detector> placement(path, bbox_, marker_trans_, detector_,
                                                     sym_.get_spacing() * scale_factor_,
                                                     sym_.get_max_error(),
                                                     sym_.get_allow_overlap());
            double x = 0;
            double y = 0;
            double angle = 0;
            while (placement.get_point(x, y, angle))
            {
                agg::trans_affine matrix = marker_trans_;
                matrix.rotate(angle);
                matrix.translate(x, y);
                svg_renderer_.render(ras_, sl_, renb_, matrix, sym_.get_opacity(), bbox_);
            }
        }
    }
private:
    agg::scanline_u8 sl_;
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
};

template <typename BufferType, typename Rasterizer, typename Detector>
struct raster_markers_rasterizer_dispatch
{
    typedef agg::rgba8 color_type;
    typedef agg::order_rgba order_type;
    typedef agg::pixel32_type pixel_type;
    typedef agg::comp_op_adaptor_rgba_pre<color_type, order_type> blender_type; // comp blender
    typedef agg::pixfmt_custom_blend_rgba<blender_type, BufferType> pixfmt_comp_type;
    typedef agg::renderer_base<pixfmt_comp_type> renderer_base;

    raster_markers_rasterizer_dispatch(BufferType & render_buffer,
                                       Rasterizer & ras,
                                       image_data_32 const& src,
                                       agg::trans_affine const& marker_trans,
                                       markers_symbolizer const& sym,
                                       Detector & detector,
                                       double scale_factor)
        : buf_(render_buffer),
        pixf_(buf_),
        renb_(pixf_),
        ras_(ras),
        src_(src),
        marker_trans_(marker_trans),
        sym_(sym),
        detector_(detector),
        scale_factor_(scale_factor)
    {
        pixf_.comp_op(static_cast<agg::comp_op_e>(sym_.comp_op()));
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
                render_raster_marker(matrix, sym_.get_opacity());
                if (!sym_.get_ignore_placement())
                {
                    detector_.insert(transformed_bbox);
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
                render_raster_marker(matrix, sym_.get_opacity());
            }
        }
    }

    void render_raster_marker(agg::trans_affine const& marker_tr,
                              double opacity)
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
        agg::span_allocator<color_type> sa;
        agg::image_filter_bilinear filter_kernel;
        agg::image_filter_lut filter(filter_kernel, false);
        agg::rendering_buffer marker_buf((unsigned char *)src_.getBytes(),
                                         src_.width(),
                                         src_.height(),
                                         src_.width()*4);
        agg::pixfmt_rgba32_pre pixf(marker_buf);
        typedef agg::image_accessor_clone<agg::pixfmt_rgba32_pre> img_accessor_type;
        typedef agg::span_interpolator_linear<agg::trans_affine> interpolator_type;
        typedef agg::span_image_filter_rgba_2x2<img_accessor_type,
                                                interpolator_type> span_gen_type;
        typedef agg::renderer_scanline_aa_alpha<renderer_base,
                agg::span_allocator<color_type>,
                span_gen_type> renderer_type;
        img_accessor_type ia(pixf);
        interpolator_type interpolator(agg::trans_affine(p, 0, 0, width, height) );
        span_gen_type sg(ia, interpolator, filter);
        renderer_type rp(renb_,sa, sg, unsigned(opacity*255));
        agg::render_scanlines(ras_, sl_, rp);
    }

private:
    agg::scanline_u8 sl_;
    BufferType & buf_;
    pixfmt_comp_type pixf_;
    renderer_base renb_;
    Rasterizer & ras_;
    image_data_32 const& src_;
    agg::trans_affine const& marker_trans_;
    markers_symbolizer const& sym_;
    Detector & detector_;
    double scale_factor_;
};


template <typename T>
void build_ellipse(T const& sym, mapnik::feature_impl const& feature, svg_storage_type & marker_ellipse, svg::svg_path_adapter & svg_path)
{
    expression_ptr const& width_expr = sym.get_width();
    expression_ptr const& height_expr = sym.get_height();
    double width = 0;
    double height = 0;
    if (width_expr && height_expr)
    {
        width = boost::apply_visitor(evaluate<Feature,value_type>(feature), *width_expr).to_double();
        height = boost::apply_visitor(evaluate<Feature,value_type>(feature), *height_expr).to_double();
    }
    else if (width_expr)
    {
        width = boost::apply_visitor(evaluate<Feature,value_type>(feature), *width_expr).to_double();
        height = width;
    }
    else if (height_expr)
    {
        height = boost::apply_visitor(evaluate<Feature,value_type>(feature), *height_expr).to_double();
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
    marker_ellipse.set_bounding_box(lox,loy,hix,hiy);
}

template <typename Attr>
bool push_explicit_style(Attr const& src, Attr & dst, markers_symbolizer const& sym)
{
    boost::optional<stroke> const& strk = sym.get_stroke();
    boost::optional<color> const& fill = sym.get_fill();
    boost::optional<float> const& fill_opacity = sym.get_fill_opacity();
    if (strk || fill || fill_opacity)
    {
        bool success = false;
        for(unsigned i = 0; i < src.size(); ++i)
        {
            success = true;
            dst.push_back(src[i]);
            mapnik::svg::path_attributes & attr = dst.last();
            if (attr.stroke_flag)
            {
                // TODO - stroke attributes need to be boost::optional
                // for this to work properly
                if (strk)
                {
                    attr.stroke_width = strk->get_width();
                    color const& s_color = strk->get_color();
                    attr.stroke_color = agg::rgba(s_color.red()/255.0,
                                                  s_color.green()/255.0,
                                                  s_color.blue()/255.0,
                                                  s_color.alpha()/255.0);
                    attr.stroke_opacity = strk->get_opacity();
                }
            }
            if (attr.fill_flag)
            {
                if (fill)
                {
                    color const& f_color = *fill;
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
void setup_transform_scaling(agg::trans_affine & tr, box2d<double> const& bbox, mapnik::feature_impl const& feature, T const& sym)
{
    double width = 0;
    double height = 0;

    expression_ptr const& width_expr = sym.get_width();
    if (width_expr)
        width = boost::apply_visitor(evaluate<Feature,value_type>(feature), *width_expr).to_double();

    expression_ptr const& height_expr = sym.get_height();
    if (height_expr)
        height = boost::apply_visitor(evaluate<Feature,value_type>(feature), *height_expr).to_double();

    if (width > 0 && height > 0)
    {
        double sx = width/bbox.width();
        double sy = height/bbox.height();
        tr *= agg::trans_affine_scaling(sx,sy);
    }
    else if (width > 0)
    {
        double sx = width/bbox.width();
        tr *= agg::trans_affine_scaling(sx);
    }
    else if (height > 0)
    {
        double sy = height/bbox.height();
        tr *= agg::trans_affine_scaling(sy);
    }
}

}

#endif //MAPNIK_MARKER_HELPERS_HPP
