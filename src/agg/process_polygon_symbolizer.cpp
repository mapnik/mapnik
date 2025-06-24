/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/renderer_common/process_polygon_symbolizer.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_renderer_scanline.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

template<typename T0, typename T1>
void agg_renderer<T0, T1>::process(polygon_symbolizer const& sym,
                                   mapnik::feature_impl& feature,
                                   proj_transform const& prj_trans)
{
    using vertex_converter_type =
      vertex_converter<clip_poly_tag, transform_tag, affine_transform_tag, simplify_tag, smooth_tag>;

    ras_ptr->reset();
    const double gamma = get<value_double>(sym, keys::gamma, feature, common_.vars_, 1.0);
    gamma_method_enum gamma_method =
      get<gamma_method_enum>(sym, keys::gamma_method, feature, common_.vars_, gamma_method_enum::GAMMA_POWER);
    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }

    buffer_type& current_buffer = buffers_.top().get();
    agg::rendering_buffer buf(current_buffer.bytes(),
                              current_buffer.width(),
                              current_buffer.height(),
                              current_buffer.row_size());

    box2d<double> clip_box = clipping_extent(common_);
    render_polygon_symbolizer<vertex_converter_type>(
      sym,
      feature,
      prj_trans,
      common_,
      clip_box,
      *ras_ptr,
      [&](color const& fill, double opacity) {
          unsigned r = fill.red();
          unsigned g = fill.green();
          unsigned b = fill.blue();
          unsigned a = fill.alpha();
          using color_type = agg::rgba8;
          using order_type = agg::order_rgba;
          using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>; // comp blender
          using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
          using renderer_base = agg::renderer_base<pixfmt_comp_type>;
          using renderer_type = agg::renderer_scanline_aa_solid<renderer_base>;
          pixfmt_comp_type pixf(buf);
          pixf.comp_op(
            static_cast<agg::comp_op_e>(get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over)));
          renderer_base renb(pixf);
          renderer_type ren(renb);
          ren.color(agg::rgba8_pre(r, g, b, int(a * opacity)));
          agg::scanline_u8 sl;
          ras_ptr->filling_rule(agg::fill_even_odd);
          agg::render_scanlines(*ras_ptr, sl, ren);
      });
}

template void
  agg_renderer<image_rgba8>::process(polygon_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik
