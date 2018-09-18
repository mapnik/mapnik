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

#ifndef MAPNIK_CAIRO_RENDER_POLYGON_PATTERN_HPP
#define MAPNIK_CAIRO_RENDER_POLYGON_PATTERN_HPP

#include <mapnik/symbolizer.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/renderer_common/pattern_alignment.hpp>
#include <mapnik/renderer_common/render_pattern.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/marker.hpp>

namespace mapnik {

struct cairo_renderer_process_visitor_p
{
    cairo_renderer_process_visitor_p(cairo_context & context,
                                     agg::trans_affine & image_tr,
                                     unsigned offset_x,
                                     unsigned offset_y,
                                     float opacity)
        : context_(context),
          image_tr_(image_tr),
          offset_x_(offset_x),
          offset_y_(offset_y),
          opacity_(opacity) {}

    void operator() (marker_null const&) {}

    void operator() (marker_svg const& marker)
    {
        mapnik::rasterizer ras;
        mapnik::box2d<double> const& bbox_image = marker.get_data()->bounding_box() * image_tr_;
        mapnik::image_rgba8 image(bbox_image.width(), bbox_image.height());
        render_pattern<image_rgba8>(ras, marker, image_tr_, 1.0, image);
        cairo_pattern pattern(image, opacity_);
        pattern.set_extend(CAIRO_EXTEND_REPEAT);
        pattern.set_origin(offset_x_, offset_y_);
        context_.set_pattern(pattern);
    }

    void operator() (marker_rgba8 const& marker)
    {
        cairo_pattern pattern(marker.get_data(), opacity_);
        pattern.set_extend(CAIRO_EXTEND_REPEAT);
        pattern.set_origin(offset_x_, offset_y_);
        context_.set_pattern(pattern);
    }

  private:
    cairo_context & context_;
    agg::trans_affine & image_tr_;
    unsigned offset_x_;
    unsigned offset_y_;
    float opacity_;
};

struct cairo_pattern_base
{
    mapnik::marker const& marker_;
    renderer_common const& common_;
    symbolizer_base const& sym_;
    mapnik::feature_impl const& feature_;
    proj_transform const& prj_trans_;

    agg::trans_affine geom_transform() const
    {
        agg::trans_affine tr;
        auto transform = get_optional<transform_type>(sym_, keys::geometry_transform);
        if (transform)
        {
            evaluate_transform(tr, feature_, common_.vars_, *transform, common_.scale_factor_);
        }
        return tr;
    }
};

template <typename VertexConverter>
struct cairo_polygon_pattern : cairo_pattern_base
{
    cairo_polygon_pattern(mapnik::marker const& marker,
                          renderer_common const& common,
                          symbolizer_base const& sym,
                          mapnik::feature_impl const& feature,
                          proj_transform const& prj_trans)
        : cairo_pattern_base{marker, common, sym, feature, prj_trans},
          clip_(get<value_bool, keys::clip>(sym_, feature_, common_.vars_)),
          clip_box_(clipping_extent(common)),
          tr_(geom_transform()),
          converter_(clip_box_, sym, common.t_, prj_trans, tr_,
                     feature, common.vars_, common.scale_factor_)
    {
        value_double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym, feature, common_.vars_);
        value_double smooth = get<value_double, keys::smooth>(sym, feature, common_.vars_);

        converter_.template set<affine_transform_tag>();
        if (simplify_tolerance > 0.0) converter_.template set<simplify_tag>();
        if (smooth > 0.0) converter_.template set<smooth_tag>();
    }

    void render(cairo_fill_rule_t fill_rule, cairo_context & context)
    {
        unsigned offset_x=0;
        unsigned offset_y=0;
        pattern_alignment_enum alignment = get<pattern_alignment_enum, keys::alignment>(sym_, feature_, common_.vars_);
        if (alignment == LOCAL_ALIGNMENT)
        {
            double x0 = 0.0;
            double y0 = 0.0;
            using apply_local_alignment = detail::apply_local_alignment;
            apply_local_alignment apply(common_.t_, prj_trans_, clip_box_, x0, y0);
            util::apply_visitor(geometry::vertex_processor<apply_local_alignment>(apply), feature_.get_geometry());
            offset_x = std::abs(clip_box_.width() - x0);
            offset_y = std::abs(clip_box_.height() - y0);
        }

        value_double opacity = get<value_double, keys::opacity>(sym_, feature_, common_.vars_);
        agg::trans_affine image_tr = agg::trans_affine_scaling(common_.scale_factor_);
        auto image_transform = get_optional<transform_type>(sym_, keys::image_transform);
        if (image_transform)
        {
            evaluate_transform(image_tr, feature_, common_.vars_, *image_transform, common_.scale_factor_);
        }

        composite_mode_e comp_op = get<composite_mode_e, keys::comp_op>(sym_, feature_, common_.vars_);

        cairo_save_restore guard(context);
        context.set_operator(comp_op);

        util::apply_visitor(cairo_renderer_process_visitor_p(
            context, image_tr, offset_x, offset_y, opacity), marker_);

        using apply_vertex_converter_type = detail::apply_vertex_converter<VertexConverter, cairo_context>;
        using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
        apply_vertex_converter_type apply(converter_, context);
        mapnik::util::apply_visitor(vertex_processor_type(apply),feature_.get_geometry());
        // fill polygon
        context.set_fill_rule(fill_rule);
        context.fill();
    }

    const bool clip_;
    const box2d<double> clip_box_;
    const agg::trans_affine tr_;
    VertexConverter converter_;
};

} // namespace mapnik


#endif // MAPNIK_CAIRO_RENDER_POLYGON_PATTERN_HPP
