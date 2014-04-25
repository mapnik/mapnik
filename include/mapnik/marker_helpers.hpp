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
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/marker.hpp> // for svg_storage_type
#include <mapnik/svg/svg_storage.hpp>
#include <mapnik/markers_placement.hpp>

// agg
#include "agg_ellipse.h"
#include "agg_basics.h"
#include "agg_color_rgba.h"
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
#include "agg_span_interpolator_linear.h"

// boost
#include <boost/optional.hpp>
#include <boost/variant/apply_visitor.hpp>

namespace mapnik {


template <typename BufferType, typename SvgRenderer, typename Rasterizer, typename Detector>
struct vector_markers_rasterizer_dispatch
{
    typedef typename SvgRenderer::renderer_base         renderer_base;
    typedef typename SvgRenderer::vertex_source_type    vertex_source_type;
    typedef typename SvgRenderer::attribute_source_type attribute_source_type;
    typedef typename renderer_base::pixfmt_type         pixfmt_type;
  
    vector_markers_rasterizer_dispatch(BufferType & render_buffer,
                                       vertex_source_type &path,
                                       attribute_source_type const &attrs,
                                       Rasterizer & ras,
                                       box2d<double> const& bbox,
                                       agg::trans_affine const& marker_trans,
                                       markers_symbolizer const& sym,
                                       Detector & detector,
                                       double scale_factor,
                                       bool snap_to_pixels)
    : buf_(render_buffer),
        pixf_(buf_),
        renb_(pixf_),
        svg_renderer_(path, attrs),
        ras_(ras),
        bbox_(bbox),
        marker_trans_(marker_trans),
        sym_(sym),
        detector_(detector),
        scale_factor_(scale_factor),
        snap_to_pixels_(snap_to_pixels)
    {
        pixf_.comp_op(get<agg::comp_op_e>(sym_, keys::comp_op, agg::comp_op_src_over));
    }

    vector_markers_rasterizer_dispatch(vector_markers_rasterizer_dispatch &&d)
      : buf_(d.buf_), pixf_(d.pixf_), svg_renderer_(std::move(d.svg_renderer_)), ras_(d.ras_),
        bbox_(d.bbox_), marker_trans_(d.marker_trans_), sym_(d.sym_), detector_(d.detector_),
        scale_factor_(d.scale_factor_), snap_to_pixels_(d.snap_to_pixels_)
    {
    }

    template <typename T>
    void add_path(T & path)
    {
        marker_placement_enum placement_method = get<marker_placement_enum>(sym_, keys::markers_placement_type, MARKER_POINT_PLACEMENT);
        bool ignore_placement = get<bool>(sym_, keys::ignore_placement, false);
        bool allow_overlap = get<bool>(sym_, keys::allow_overlap, false);
        double opacity = get<double>(sym_,keys::opacity, 1.0);

        if (placement_method != MARKER_LINE_PLACEMENT ||
            path.type() == mapnik::geometry_type::types::Point)
        {
            double x = 0;
            double y = 0;
            if (path.type() == mapnik::geometry_type::types::LineString)
            {
                if (!label::middle_point(path, x, y))
                    return;
            }
            else if (placement_method == MARKER_INTERIOR_PLACEMENT)
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
            if (snap_to_pixels_)
            {
                // https://github.com/mapnik/mapnik/issues/1316
                matrix.tx = std::floor(matrix.tx+.5);
                matrix.ty = std::floor(matrix.ty+.5);
            }
            // TODO https://github.com/mapnik/mapnik/issues/1754
            box2d<double> transformed_bbox = bbox_ * matrix;

            if (allow_overlap ||
                detector_.has_placement(transformed_bbox))
            {
                svg_renderer_.render(ras_, sl_, renb_, matrix, opacity, bbox_);
                if (!ignore_placement)
                {
                    detector_.insert(transformed_bbox);
                }
            }
        }
        else
        {
            double spacing = get<double>(sym_, keys::spacing, 100.0);
            double max_error = get<double>(sym_, keys::max_error, 0.2);
            markers_placement<T, Detector> placement(path, bbox_, marker_trans_, detector_,
                                                     spacing * scale_factor_,
                                                     max_error,
                                                     allow_overlap);
            double x = 0;
            double y = 0;
            double angle = 0;
            while (placement.get_point(x, y, angle, ignore_placement))
            {
                agg::trans_affine matrix = marker_trans_;
                matrix.rotate(angle);
                matrix.translate(x, y);
                svg_renderer_.render(ras_, sl_, renb_, matrix, opacity, bbox_);
            }
        }
    }
private:
    agg::scanline_u8 sl_;
    BufferType & buf_;
    pixfmt_type pixf_;
    renderer_base renb_;
    SvgRenderer svg_renderer_;
    Rasterizer & ras_;
    box2d<double> const& bbox_;
    agg::trans_affine const& marker_trans_;
    markers_symbolizer const& sym_;
    Detector & detector_;
    double scale_factor_;
    bool snap_to_pixels_;
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
                                       double scale_factor,
                                       bool snap_to_pixels)
        : buf_(render_buffer),
        pixf_(buf_),
        renb_(pixf_),
        ras_(ras),
        src_(src),
        marker_trans_(marker_trans),
        sym_(sym),
        detector_(detector),
        scale_factor_(scale_factor),
        snap_to_pixels_(snap_to_pixels)
    {
        pixf_.comp_op(get<agg::comp_op_e>(sym_, keys::comp_op, agg::comp_op_src_over));
    }

    raster_markers_rasterizer_dispatch(raster_markers_rasterizer_dispatch &&d) 
      : buf_(d.buf_), pixf_(d.pixf_), renb_(d.renb_), ras_(d.ras_), src_(d.src_), 
        marker_trans_(d.marker_trans_), sym_(d.sym_), detector_(d.detector_), 
        scale_factor_(d.scale_factor_), snap_to_pixels_(d.snap_to_pixels_)
    {
    }

    template <typename T>
    void add_path(T & path)
    {
        marker_placement_enum placement_method = get<marker_placement_enum>(sym_, keys::markers_placement_type, MARKER_POINT_PLACEMENT);
        bool allow_overlap = get<bool>(sym_, keys::allow_overlap, false);
        box2d<double> bbox_(0,0, src_.width(),src_.height());
        double opacity = get<double>(sym_, keys::opacity, 1.0);
        bool ignore_placement = get<bool>(sym_, keys::ignore_placement, false);

        if (placement_method != MARKER_LINE_PLACEMENT ||
            path.type() == mapnik::geometry_type::types::Point)
        {
            double x = 0;
            double y = 0;
            if (path.type() == mapnik::geometry_type::types::LineString)
            {
                if (!label::middle_point(path, x, y))
                    return;
            }
            else if (placement_method == MARKER_INTERIOR_PLACEMENT)
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

            if (allow_overlap ||
                detector_.has_placement(transformed_bbox))
            {
                render_raster_marker(matrix, opacity);
                if (!ignore_placement)
                {
                    detector_.insert(transformed_bbox);
                }
            }
        }
        else
        {
            double spacing  = get<double>(sym_, keys::spacing, 100.0);
            double max_error  = get<double>(sym_, keys::max_error, 0.2);
            markers_placement<T, label_collision_detector4> placement(path, bbox_, marker_trans_, detector_,
                                                                      spacing * scale_factor_,
                                                                      max_error,
                                                                      allow_overlap);
            double x, y, angle;
            while (placement.get_point(x, y, angle, ignore_placement))
            {
                agg::trans_affine matrix = marker_trans_;
                matrix.rotate(angle);
                matrix.translate(x, y);
                render_raster_marker(matrix, opacity);
            }
        }
    }

    void render_raster_marker(agg::trans_affine const& marker_tr,
                              double opacity)
    {
        typedef agg::pixfmt_rgba32_pre pixfmt_pre;
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
            renb_.blend_from(pixf_mask,
                             0,
                             std::floor(marker_tr.tx + .5),
                             std::floor(marker_tr.ty + .5),
                             unsigned(255*opacity));
        }
        else
        {
            typedef agg::image_accessor_clone<pixfmt_pre> img_accessor_type;
            typedef agg::span_interpolator_linear<> interpolator_type;
            //typedef agg::span_image_filter_rgba_2x2<img_accessor_type,interpolator_type> span_gen_type;
            typedef agg::span_image_resample_rgba_affine<img_accessor_type> span_gen_type;
            typedef agg::renderer_scanline_aa_alpha<renderer_base,
                    agg::span_allocator<color_type>,
                    span_gen_type> renderer_type;

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
    bool snap_to_pixels_;
};


template <typename T>
void build_ellipse(T const& sym, mapnik::feature_impl const& feature, svg_storage_type & marker_ellipse, svg::svg_path_adapter & svg_path)
{
    auto width_expr  = get<expression_ptr>(sym, keys::width);
    auto height_expr = get<expression_ptr>(sym, keys::height);
    double width = 0;
    double height = 0;
    if (width_expr && height_expr)
    {
        width = boost::apply_visitor(evaluate<feature_impl,value_type>(feature), *width_expr).to_double();
        height = boost::apply_visitor(evaluate<feature_impl,value_type>(feature), *height_expr).to_double();
    }
    else if (width_expr)
    {
        width = boost::apply_visitor(evaluate<feature_impl,value_type>(feature), *width_expr).to_double();
        height = width;
    }
    else if (height_expr)
    {
        height = boost::apply_visitor(evaluate<feature_impl,value_type>(feature), *height_expr).to_double();
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
bool push_explicit_style(Attr const& src, Attr & dst, markers_symbolizer const& sym)
{
    auto fill_color = get_optional<color>(sym, keys::fill);
    auto fill_opacity = get_optional<double>(sym, keys::fill_opacity);
    auto stroke_color = get_optional<color>(sym, keys::stroke);
    auto stroke_width = get_optional<double>(sym, keys::stroke_width);
    auto stroke_opacity = get_optional<double>(sym, keys::stroke_opacity);
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
                             mapnik::feature_impl const& feature,
                             T const& sym)
{
    double width = 0;
    double height = 0;

    expression_ptr width_expr = get<expression_ptr>(sym, keys::width);
    if (width_expr)
        width = boost::apply_visitor(evaluate<feature_impl,value_type>(feature), *width_expr).to_double();

    expression_ptr height_expr = get<expression_ptr>(sym, keys::height);
    if (height_expr)
        height = boost::apply_visitor(evaluate<feature_impl,value_type>(feature), *height_expr).to_double();

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
void apply_markers_multi(feature_impl & feature, Converter& converter, markers_symbolizer const& sym)
{
  std::size_t geom_count = feature.paths().size();
  if (geom_count == 1)
  {
      converter.apply(feature.paths()[0]);
  }
  else if (geom_count > 1)
  {
      marker_multi_policy_enum multi_policy = get<marker_multi_policy_enum>(sym, keys::markers_multipolicy, MARKER_EACH_MULTI);
      marker_placement_enum placement = get<marker_placement_enum>(sym, keys::markers_placement_type, MARKER_POINT_PLACEMENT);
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
          geometry_type* largest = 0;
          for (geometry_type & geom : feature.paths())
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
          for (geometry_type & path : feature.paths())
          {
            converter.apply(path);
          }
      }
  }
}

}

#endif //MAPNIK_MARKER_HELPERS_HPP
