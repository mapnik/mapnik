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

// boost


// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/renderer_common/process_polygon_symbolizer.hpp>

// agg
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_renderer_scanline.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"

namespace mapnik {

template <typename T0, typename T1>
void agg_renderer<T0,T1>::process(polygon_symbolizer const& sym,
                              mapnik::feature_impl & feature,
                              proj_transform const& prj_trans)
{
    typedef boost::mpl::vector<clip_poly_tag,transform_tag,affine_transform_tag,simplify_tag,smooth_tag> conv_types;
    typedef vertex_converter<box2d<double>, rasterizer, polygon_symbolizer,
                             CoordTransform, proj_transform, agg::trans_affine, 
                             conv_types, feature_impl> vertex_converter_type;

    ras_ptr->reset();
    double gamma = get<value_double>(sym, keys::gamma, feature, 1.0);
    gamma_method_enum gamma_method = get<gamma_method_enum>(sym, keys::gamma_method, feature, GAMMA_POWER);
    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }

    box2d<double> clip_box = clipping_extent();
    agg::rendering_buffer buf(current_buffer_->raw_data(),current_buffer_->width(),current_buffer_->height(), current_buffer_->width() * 4);

    render_polygon_symbolizer<vertex_converter_type>(
        sym, feature, prj_trans, common_, clip_box, *ras_ptr,
        [&](color const &fill, double opacity) {
            unsigned r=fill.red();
            unsigned g=fill.green();
            unsigned b=fill.blue();
            unsigned a=fill.alpha();
            typedef agg::rgba8 color_type;
            typedef agg::order_rgba order_type;
            typedef agg::comp_op_adaptor_rgba_pre<color_type, order_type> blender_type; // comp blender
            typedef agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer> pixfmt_comp_type;
            typedef agg::renderer_base<pixfmt_comp_type> renderer_base;
            typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_type;
            pixfmt_comp_type pixf(buf);
            pixf.comp_op(get<agg::comp_op_e>(sym, keys::comp_op, feature, agg::comp_op_src_over));
            renderer_base renb(pixf);
            renderer_type ren(renb);
            ren.color(agg::rgba8_pre(r, g, b, int(a * opacity)));
            agg::scanline_u8 sl;
            ras_ptr->filling_rule(agg::fill_even_odd);
            agg::render_scanlines(*ras_ptr, sl, ren);
        });
}

template void agg_renderer<image_32>::process(polygon_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
