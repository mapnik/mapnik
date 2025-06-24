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

// mapnik
#include <mapnik/text/color_font_renderer.hpp>
#include <mapnik/text/renderer.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/text/face.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/image_util.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rendering_buffer.h"
#include "agg_color_rgba.h"
#include "agg_scanline_u.h"
#include "agg_image_filters.h"
#include "agg_trans_bilinear.h"
#include "agg_span_allocator.h"
#include "agg_image_accessors.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_pixfmt_rgba.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

template<typename Pixmap, typename ImageAccessor>
void composite_image(Pixmap& pixmap,
                     ImageAccessor& img_accessor,
                     double width,
                     double height,
                     agg::trans_affine const& tr,
                     double opacity,
                     composite_mode_e comp_op)
{
    double p[8];
    p[0] = 0;
    p[1] = 0;
    p[2] = width;
    p[3] = 0;
    p[4] = width;
    p[5] = height;
    p[6] = 0;
    p[7] = height;

    tr.transform(&p[0], &p[1]);
    tr.transform(&p[2], &p[3]);
    tr.transform(&p[4], &p[5]);
    tr.transform(&p[6], &p[7]);

    rasterizer ras;
    ras.move_to_d(p[0], p[1]);
    ras.line_to_d(p[2], p[3]);
    ras.line_to_d(p[4], p[5]);
    ras.line_to_d(p[6], p[7]);

    using color_type = agg::rgba8;
    using order_type = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>;
    using pixfmt_comp_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
    using renderer_base = agg::renderer_base<pixfmt_comp_type>;

    agg::scanline_u8 sl;
    agg::rendering_buffer buf(pixmap.bytes(), pixmap.width(), pixmap.height(), pixmap.row_size());
    pixfmt_comp_type pixf(buf);
    pixf.comp_op(static_cast<agg::comp_op_e>(comp_op));
    renderer_base renb(pixf);

    agg::span_allocator<color_type> sa;
    agg::image_filter_bilinear filter_kernel;
    agg::image_filter_lut filter(filter_kernel, false);

    using interpolator_type = agg::span_interpolator_linear<agg::trans_affine>;
    using span_gen_type = agg::span_image_resample_rgba_affine<ImageAccessor>;
    using renderer_type =
      agg::renderer_scanline_aa_alpha<renderer_base, agg::span_allocator<agg::rgba8>, span_gen_type>;
    agg::trans_affine final_tr(p, 0, 0, width, height);
    final_tr.tx = std::floor(final_tr.tx + .5);
    final_tr.ty = std::floor(final_tr.ty + .5);
    interpolator_type interpolator(final_tr);
    span_gen_type sg(img_accessor, interpolator, filter);
    renderer_type rp(renb, sa, sg, static_cast<unsigned>(opacity * 255));
    agg::render_scanlines(ras, sl, rp);
}

agg::trans_affine glyph_transform(agg::trans_affine const& tr,
                                  unsigned glyph_height,
                                  int x,
                                  int y,
                                  double angle,
                                  box2d<double> const& bbox)
{
    agg::trans_affine t;
    double scale = bbox.height() / glyph_height;
    t *= agg::trans_affine_translation(0, -bbox.maxy() / scale); // set to baseline
    t *= tr;
    t *= agg::trans_affine_rotation(angle);
    t *= agg::trans_affine_scaling(scale);
    t *= agg::trans_affine_translation(x, y);
    return t;
}

template<typename T>
void composite_color_glyph(T& pixmap,
                           FT_Bitmap const& bitmap,
                           agg::trans_affine const& tr,
                           double opacity,
                           composite_mode_e comp_op)
{
    using glyph_pixfmt_type = agg::pixfmt_bgra32_pre;
    using img_accessor_type = agg::image_accessor_clone<glyph_pixfmt_type>;
    unsigned width = bitmap.width;
    unsigned height = bitmap.rows;
    agg::rendering_buffer glyph_buf(bitmap.buffer, width, height, width * glyph_pixfmt_type::pix_width);
    glyph_pixfmt_type glyph_pixf(glyph_buf);
    img_accessor_type img_accessor(glyph_pixf);

    composite_image<T, img_accessor_type>(pixmap, img_accessor, width, height, tr, opacity, comp_op);
}

template void composite_color_glyph<image_rgba8>(image_rgba8& pixmap,
                                                 FT_Bitmap const& bitmap,
                                                 agg::trans_affine const& tr,
                                                 double opacity,
                                                 composite_mode_e comp_op);

image_rgba8 render_glyph_image(glyph_t const& glyph,
                               FT_Bitmap const& bitmap,
                               agg::trans_affine const& tr,
                               pixel_position& render_pos)
{
    agg::trans_affine transform(glyph_transform(tr, bitmap.rows, 0, 0, -glyph.rot.angle(), glyph.bbox));
    box2d<double> bitmap_bbox(0, 0, bitmap.width, bitmap.rows);
    bitmap_bbox *= transform;
    image_rgba8 glyph_image(bitmap_bbox.width(), bitmap_bbox.height());
    transform *= agg::trans_affine_translation(-bitmap_bbox.minx(), -bitmap_bbox.miny());
    render_pos = render_pos + pixel_position(glyph.pos.x, -glyph.pos.y) +
                 pixel_position(std::round(bitmap_bbox.minx()), std::round(bitmap_bbox.miny()));
    composite_color_glyph(glyph_image, bitmap, transform, 1, dst_over);
    return glyph_image;
}

} // namespace mapnik
