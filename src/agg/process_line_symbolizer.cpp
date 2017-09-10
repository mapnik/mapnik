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

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>
#include <mapnik/geometry/geometry_type.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_p.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_renderer_outline_aa.h"
#include "agg_rasterizer_outline_aa.h"
#pragma GCC diagnostic pop

// stl
#include <string>
#include <cmath>

namespace mapnik {

template <typename Symbolizer, typename Rasterizer, typename Feature>
void set_join_caps_aa(Symbolizer const& sym, Rasterizer & ras, Feature & feature, attributes const& vars)
{
    line_join_enum join = get<line_join_enum, keys::stroke_linejoin>(sym, feature, vars);
    switch (join)
    {
    case MITER_JOIN:
        ras.line_join(agg::outline_miter_accurate_join);
        break;
    case MITER_REVERT_JOIN:
        ras.line_join(agg::outline_no_join);
        break;
    case ROUND_JOIN:
        ras.line_join(agg::outline_round_join);
        break;
    default:
        ras.line_join(agg::outline_no_join);
    }

    line_cap_enum cap = get<line_cap_enum, keys::stroke_linecap>(sym, feature, vars);

    switch (cap)
    {
    case BUTT_CAP:
        ras.round_cap(false);
        break;
    case SQUARE_CAP:
        ras.round_cap(false);
        break;
    default:
        ras.round_cap(true);
    }
}

template <typename T0, typename T1>
void agg_renderer<T0,T1>::process(line_symbolizer const& sym,
                              mapnik::feature_impl & feature,
                              proj_transform const& prj_trans)

{
    color const& col = get<color, keys::stroke>(sym, feature, common_.vars_);
    unsigned r=col.red();
    unsigned g=col.green();
    unsigned b=col.blue();
    unsigned a=col.alpha();

    double gamma = get<value_double, keys::stroke_gamma>(sym, feature, common_.vars_);
    gamma_method_enum gamma_method = get<gamma_method_enum, keys::stroke_gamma_method>(sym, feature, common_.vars_);
    ras_ptr->reset();

    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }

    agg::rendering_buffer buf(current_buffer_->bytes(),current_buffer_->width(),current_buffer_->height(), current_buffer_->row_size());

    using color_type = agg::rgba8;
    using order_type = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>; // comp blender
    using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
    using renderer_base = agg::renderer_base<pixfmt_comp_type>;

    pixfmt_comp_type pixf(buf);
    pixf.comp_op(static_cast<agg::comp_op_e>(get<composite_mode_e, keys::comp_op>(sym, feature, common_.vars_)));
    renderer_base renb(pixf);

    agg::trans_affine tr;
    auto transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (transform) evaluate_transform(tr, feature, common_.vars_, *transform, common_.scale_factor_);

    box2d<double> clip_box = clipping_extent(common_);

    value_bool clip = get<value_bool, keys::clip>(sym, feature, common_.vars_);
    value_double width = get<value_double, keys::stroke_width>(sym, feature, common_.vars_);
    value_double opacity = get<value_double,keys::stroke_opacity>(sym,feature, common_.vars_);
    value_double offset = get<value_double, keys::offset>(sym, feature, common_.vars_);
    value_double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym, feature, common_.vars_);
    value_double smooth = get<value_double, keys::smooth>(sym, feature, common_.vars_);
    line_rasterizer_enum rasterizer_e = get<line_rasterizer_enum, keys::line_rasterizer>(sym, feature, common_.vars_);
    if (clip)
    {
        double padding = static_cast<double>(common_.query_extent_.width()/pixmap_.width());
        double half_stroke = 0.5 * width;
        if (half_stroke > 1)
        {
            padding *= half_stroke;
        }
        if (std::fabs(offset) > 0)
        {
            padding *= std::fabs(offset) * 1.2;
        }

        padding *= common_.scale_factor_;
        clip_box.pad(padding);
        // debugging
        //box2d<double> inverse = query_extent_;
        //inverse.pad(-padding);
        //draw_geo_extent(inverse,mapnik::color("red"));
    }

    if (rasterizer_e == RASTERIZER_FAST)
    {
        using renderer_type = agg::renderer_outline_aa<renderer_base>;
        using rasterizer_type = agg::rasterizer_outline_aa<renderer_type>;
        agg::line_profile_aa profile(width * common_.scale_factor_, agg::gamma_power(gamma));
        renderer_type ren(renb, profile);
        ren.color(agg::rgba8_pre(r, g, b, int(a * opacity)));
        rasterizer_type ras(ren);
        set_join_caps_aa(sym, ras, feature, common_.vars_);

        using vertex_converter_type = vertex_converter<clip_line_tag, clip_poly_tag, transform_tag,
                                                       affine_transform_tag,
                                                       simplify_tag, smooth_tag,
                                                       offset_transform_tag>;
        vertex_converter_type converter(clip_box,sym,common_.t_,prj_trans,tr,feature,common_.vars_,common_.scale_factor_);
        if (clip)
        {
            geometry::geometry_types type = geometry::geometry_type(feature.get_geometry());
            if (type == geometry::geometry_types::Polygon || type == geometry::geometry_types::MultiPolygon)
                converter.template set<clip_poly_tag>();
            else if (type == geometry::geometry_types::LineString || type == geometry::geometry_types::MultiLineString)
                converter.template set<clip_line_tag>();
        }
        converter.set<transform_tag>(); // always transform
        if (std::fabs(offset) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
        converter.set<affine_transform_tag>(); // optional affine transform
        if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
        if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter

        using apply_vertex_converter_type = detail::apply_vertex_converter<vertex_converter_type, rasterizer_type>;
        using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
        apply_vertex_converter_type apply(converter, ras);
        mapnik::util::apply_visitor(vertex_processor_type(apply),feature.get_geometry());
    }
    else
    {
        using vertex_converter_type = vertex_converter<clip_line_tag, clip_poly_tag, transform_tag,
                                                       affine_transform_tag,
                                                       simplify_tag, smooth_tag,
                                                       offset_transform_tag,
                                                       dash_tag, stroke_tag>;
        vertex_converter_type converter(clip_box, sym,common_.t_,prj_trans,tr,feature,common_.vars_,common_.scale_factor_);
        if (clip)
        {
            geometry::geometry_types type = geometry::geometry_type(feature.get_geometry());
            if (type == geometry::geometry_types::Polygon || type == geometry::geometry_types::MultiPolygon)
                converter.template set<clip_poly_tag>();
            else if (type == geometry::geometry_types::LineString || type == geometry::geometry_types::MultiLineString)
                converter.template set<clip_line_tag>();
        }
        converter.set<transform_tag>(); // always transform
        if (std::fabs(offset) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
        converter.set<affine_transform_tag>(); // optional affine transform
        if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
        if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter
        if (has_key(sym, keys::stroke_dasharray))
            converter.set<dash_tag>();
        converter.set<stroke_tag>(); //always stroke

        using apply_vertex_converter_type = detail::apply_vertex_converter<vertex_converter_type, rasterizer>;
        using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
        apply_vertex_converter_type apply(converter, *ras_ptr);
        mapnik::util::apply_visitor(vertex_processor_type(apply),feature.get_geometry());

        using renderer_type = agg::renderer_scanline_aa_solid<renderer_base>;
        renderer_type ren(renb);
        ren.color(agg::rgba8_pre(r, g, b, int(a * opacity)));
        agg::scanline_u8 sl;
        ras_ptr->filling_rule(agg::fill_non_zero);
        agg::render_scanlines(*ras_ptr, sl, ren);
    }
}


template void agg_renderer<image_rgba8>::process(line_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
