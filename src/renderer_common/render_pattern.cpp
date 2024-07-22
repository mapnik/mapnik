/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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
#include <mapnik/renderer_common/render_pattern.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/agg_rasterizer.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_pixfmt_gray.h"
#include "agg_color_rgba.h"
#include "agg_color_gray.h"
#include "agg_scanline_u.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

template<>
void render_pattern<image_rgba8>(marker_svg const& marker,
                                 agg::trans_affine const& tr,
                                 double opacity,
                                 image_rgba8& image)
{
    using color_type = agg::rgba8;
    using order_type = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>; // comp blender
    using buf_type = agg::rendering_buffer;
    using pixfmt = agg::pixfmt_custom_blend_rgba<blender_type, buf_type>;
    using renderer_base = agg::renderer_base<pixfmt>;
    using renderer_solid = agg::renderer_scanline_aa_solid<renderer_base>;

    agg::scanline_u8 sl;
    mapnik::box2d<double> const& bbox = marker.bounding_box() * tr;
    mapnik::coord<double, 2> c = bbox.center();
    agg::trans_affine mtx = agg::trans_affine_translation(-c.x, -c.y);
    mtx.translate(0.5 * bbox.width(), 0.5 * bbox.height());
    mtx = tr * mtx;

    agg::rendering_buffer buf(image.bytes(), image.width(), image.height(), image.row_size());
    pixfmt pixf(buf);
    renderer_base renb(pixf);

    svg::vertex_stl_adapter<svg::svg_path_storage> stl_storage(marker.get_data()->source());
    svg_path_adapter svg_path(stl_storage);
    svg::renderer_agg<svg_path_adapter, svg_attribute_type, renderer_solid, pixfmt> svg_renderer(
      svg_path,
      marker.get_data()->svg_group());
    rasterizer ras;
    svg_renderer.render(ras, sl, renb, mtx, opacity, bbox);
}

} // namespace mapnik
