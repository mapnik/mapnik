/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_COLOR_FONT_RENDERER_HPP
#define MAPNIK_COLOR_FONT_RENDERER_HPP

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include <agg_trans_affine.h>
MAPNIK_DISABLE_WARNING_POP

// freetype2
extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H
}

MAPNIK_DISABLE_WARNING_POP

#include <mapnik/text/glyph_info.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/geometry/box2d.hpp>

namespace mapnik {

agg::trans_affine glyph_transform(agg::trans_affine const& tr,
                                  unsigned glyph_height,
                                  int x,
                                  int y,
                                  double angle,
                                  box2d<double> const& bbox);

template<typename T>
void composite_color_glyph(T& pixmap,
                           FT_Bitmap const& bitmap,
                           agg::trans_affine const& tr,
                           double opacity,
                           composite_mode_e comp_op);

struct glyph_t;

image_rgba8 render_glyph_image(glyph_t const& glyph,
                               FT_Bitmap const& bitmap,
                               agg::trans_affine const& tr,
                               pixel_position& render_pos);

} // namespace mapnik

#endif
