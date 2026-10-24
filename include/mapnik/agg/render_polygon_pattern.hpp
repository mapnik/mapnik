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

#ifndef MAPNIK_RENDER_POLYGON_PATTERN_HPP
#define MAPNIK_RENDER_POLYGON_PATTERN_HPP

#include <mapnik/symbolizer.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/renderer_common/pattern_alignment.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/safe_cast.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_image_accessors.h"
#include "agg_conv_clip_polygon.h"
MAPNIK_DISABLE_WARNING_POP

// stl
#include <algorithm>
#include <cstring>

namespace mapnik {

// What agg::span_pattern_rgba over agg::image_accessor_wrap with agg::wrap_mode_repeat generates, copied from
// the pattern's rows a run at a time: agg's copies a pixel at a time, a byte per channel, and those stores may
// alias the accessor, so it reloads and steps the wrap-around state for every pixel. (The pattern's channels
// are in the order of agg::rgba8's.)
class span_pattern_rgba_repeat
{
    static_assert(sizeof(agg::rgba8) == 4 && agg::pixfmt_rgba32_pre::pix_width == 4);

  public:
    span_pattern_rgba_repeat(agg::pixfmt_rgba32_pre const& pattern, unsigned offset_x, unsigned offset_y)
        : pattern_(pattern),
          wrap_x_(pattern.width()),
          wrap_y_(pattern.height()),
          offset_x_(offset_x),
          offset_y_(offset_y)
    {}

    void prepare() {}

    void generate(agg::rgba8* span, int x, int y, unsigned len)
    {
        x += offset_x_;
        y += offset_y_;
        agg::int8u const* row = pattern_.row_ptr(wrap_y_(y));
        unsigned const width = pattern_.width();
        unsigned column = wrap_x_(x);
        while (len > 0)
        {
            unsigned const n = std::min(len, width - column);
            std::memcpy(span, row + column * sizeof(agg::rgba8), n * sizeof(agg::rgba8));
            span += n;
            len -= n;
            column = 0;
        }
    }

  private:
    agg::pixfmt_rgba32_pre const& pattern_;
    agg::wrap_mode_repeat wrap_x_;
    agg::wrap_mode_repeat wrap_y_;
    unsigned const offset_x_;
    unsigned const offset_y_;
};

struct agg_pattern_base
{
    image_rgba8 const& pattern_img_;
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

template<typename VertexConverter>
struct agg_polygon_pattern : agg_pattern_base
{
    using color_type = agg::rgba8;
    using order_type = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>;
    using pixfmt_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
    using span_gen_type = span_pattern_rgba_repeat;
    using renderer_base = agg::renderer_base<pixfmt_type>;
    using renderer_type =
      agg::renderer_scanline_aa_alpha<renderer_base, agg::span_allocator<agg::rgba8>, span_gen_type>;

    agg_polygon_pattern(image_rgba8 const& pattern_img,
                        renderer_common const& common,
                        symbolizer_base const& sym,
                        mapnik::feature_impl const& feature,
                        proj_transform const& prj_trans)
        : agg_pattern_base{pattern_img, common, sym, feature, prj_trans},
          clip_(get<value_bool, keys::clip>(sym_, feature_, common_.vars_)),
          clip_box_(clipping_extent(common)),
          tr_(geom_transform()),
          converter_(clip_box_, sym, common.t_, prj_trans, tr_, feature, common.vars_, common.scale_factor_)
    {
        value_double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym_, feature_, common_.vars_);
        value_double smooth = get<value_double, keys::smooth>(sym_, feature_, common_.vars_);

        if (simplify_tolerance > 0.0)
            converter_.template set<simplify_tag>();
        converter_.template set<affine_transform_tag>();
        if (smooth > 0.0)
            converter_.template set<smooth_tag>();
    }

    void render(renderer_base& ren_base, rasterizer& ras)
    {
        coord<double, 2> offset(
          pattern_offset(sym_, feature_, prj_trans_, common_, pattern_img_.width(), pattern_img_.height()));
        agg::rendering_buffer pattern_rbuf((agg::int8u*)pattern_img_.bytes(),
                                           pattern_img_.width(),
                                           pattern_img_.height(),
                                           pattern_img_.width() * 4);
        agg::pixfmt_rgba32_pre pixf_pattern(pattern_rbuf);
        span_gen_type sg(pixf_pattern, safe_cast<unsigned>(offset.x), safe_cast<unsigned>(offset.y));

        agg::span_allocator<agg::rgba8> sa;
        value_double opacity = get<double, keys::opacity>(sym_, feature_, common_.vars_);
        renderer_type rp(ren_base, sa, sg, unsigned(opacity * 255));

        using apply_vertex_converter_type = detail::apply_vertex_converter<VertexConverter, rasterizer>;
        using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
        apply_vertex_converter_type apply(converter_, ras);
        mapnik::util::apply_visitor(vertex_processor_type(apply), feature_.get_geometry());
        agg::render_scanlines(ras, ras.scanline(), rp);
    }

    bool const clip_;
    box2d<double> const clip_box_;
    agg::trans_affine const tr_;
    VertexConverter converter_;
};

} // namespace mapnik

#endif // MAPNIK_RENDER_POLYGON_PATTERN_HPP
