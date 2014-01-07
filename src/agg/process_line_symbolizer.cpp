/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/graphics.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_rasterizer.hpp>

#include <mapnik/symbolizer.hpp>
#include <mapnik/vertex_converters.hpp>

// agg
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

// boost


// stl
#include <string>
#include <cmath>

namespace mapnik {

template <typename T0, typename T1>
void agg_renderer<T0,T1>::process(line_symbolizer const& sym,
                              mapnik::feature_impl & feature,
                              proj_transform const& prj_trans)

{
    color const& col = get<color>(sym, keys::stroke, feature, mapnik::color(0,0,0));
    unsigned r=col.red();
    unsigned g=col.green();
    unsigned b=col.blue();
    unsigned a=col.alpha();

    double gamma = get<value_double>(sym, keys::stroke_gamma, feature, 1.0);
    gamma_method_enum gamma_method = get<gamma_method_enum>(sym, keys::stroke_gamma_method, feature, GAMMA_POWER);
    ras_ptr->reset();

    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }

    agg::rendering_buffer buf(current_buffer_->raw_data(),current_buffer_->width(),current_buffer_->height(), current_buffer_->width() * 4);

    typedef agg::rgba8 color_type;
    typedef agg::order_rgba order_type;
    typedef agg::comp_op_adaptor_rgba_pre<color_type, order_type> blender_type; // comp blender
    typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_comp_type;
    typedef agg::renderer_base<pixfmt_comp_type> renderer_base;
    typedef boost::mpl::vector<clip_line_tag, transform_tag,
                               offset_transform_tag, affine_transform_tag,
                               simplify_tag, smooth_tag, dash_tag, stroke_tag> conv_types;

    pixfmt_comp_type pixf(buf);
    pixf.comp_op(get<agg::comp_op_e>(sym, keys::comp_op, feature, agg::comp_op_src_over));
    renderer_base renb(pixf);

    agg::trans_affine tr;
    auto transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (transform) evaluate_transform(tr, feature, *transform);

    box2d<double> clip_box = clipping_extent();

    bool clip = get<value_bool>(sym, keys::clip, feature, true);
    double width = get<value_double>(sym, keys::stroke_width, feature, 1.0);
    double opacity = get<value_double>(sym,keys::stroke_opacity,feature, 1.0);
    double offset = get<value_double>(sym, keys::offset, feature, 0.0);
    double simplify_tolerance = get<value_double>(sym, keys::simplify_tolerance, feature, 0.0);
    double smooth = get<value_double>(sym, keys::smooth, feature, false);
    line_rasterizer_enum rasterizer_e = get<line_rasterizer_enum>(sym, keys::line_rasterizer, feature, RASTERIZER_FULL);
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
        typedef agg::renderer_outline_aa<renderer_base> renderer_type;
        typedef agg::rasterizer_outline_aa<renderer_type> rasterizer_type;
        agg::line_profile_aa profile(width * common_.scale_factor_, agg::gamma_power(gamma));
        renderer_type ren(renb, profile);
        ren.color(agg::rgba8_pre(r, g, b, int(a * opacity)));
        rasterizer_type ras(ren);
        set_join_caps_aa(sym , ras, feature);

        vertex_converter<box2d<double>, rasterizer_type, line_symbolizer,
                         CoordTransform, proj_transform, agg::trans_affine, conv_types>
            converter(clip_box,ras,sym,common_.t_,prj_trans,tr,common_.scale_factor_);
        if (clip) converter.set<clip_line_tag>(); // optional clip (default: true)
        converter.set<transform_tag>(); // always transform
        if (std::fabs(offset) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
        converter.set<affine_transform_tag>(); // optional affine transform
        if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
        if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter

        for (geometry_type & geom : feature.paths())
        {
            if (geom.size() > 1)
            {
                converter.apply(geom);
            }
        }
    }
    else
    {
        vertex_converter<box2d<double>, rasterizer, line_symbolizer,
                         CoordTransform, proj_transform, agg::trans_affine, conv_types>
            converter(clip_box,*ras_ptr,sym,common_.t_,prj_trans,tr,common_.scale_factor_);

        if (clip) converter.set<clip_line_tag>(); // optional clip (default: true)
        converter.set<transform_tag>(); // always transform
        if (std::fabs(offset) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
        converter.set<affine_transform_tag>(); // optional affine transform
        if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
        if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter
        if (has_key<dash_array>(sym, keys::stroke_dasharray))
            converter.set<dash_tag>();
        converter.set<stroke_tag>(); //always stroke

        for (geometry_type & geom : feature.paths())
        {
            if (geom.size() > 1)
            {
                converter.apply(geom);
            }
        }

        typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_type;
        renderer_type ren(renb);
        ren.color(agg::rgba8_pre(r, g, b, int(a * opacity)));
        agg::scanline_u8 sl;
        ras_ptr->filling_rule(agg::fill_non_zero);
        agg::render_scanlines(*ras_ptr, sl, ren);
    }
}


template void agg_renderer<image_32>::process(line_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
