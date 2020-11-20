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

namespace mapnik {

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

template <typename VertexConverter>
struct agg_polygon_pattern : agg_pattern_base
{
    using color_type = agg::rgba8;
    using order_type = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>;
    using pixfmt_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
    using wrap_x_type = agg::wrap_mode_repeat;
    using wrap_y_type = agg::wrap_mode_repeat;
    using img_source_type = agg::image_accessor_wrap<agg::pixfmt_rgba32_pre,
                                                     wrap_x_type,
                                                     wrap_y_type>;
    using span_gen_type = agg::span_pattern_rgba<img_source_type>;
    using renderer_base = agg::renderer_base<pixfmt_type>;
    using renderer_type = agg::renderer_scanline_aa_alpha<renderer_base,
                                                          agg::span_allocator<agg::rgba8>,
                                                          span_gen_type>;

    agg_polygon_pattern(image_rgba8 const& pattern_img,
                        renderer_common const& common,
                        symbolizer_base const& sym,
                        mapnik::feature_impl const& feature,
                        proj_transform const& prj_trans)
        : agg_pattern_base{pattern_img, common, sym, feature, prj_trans},
          clip_(get<value_bool, keys::clip>(sym_, feature_, common_.vars_)),
          clip_box_(clipping_extent(common)),
          tr_(geom_transform()),
          converter_(clip_box_, sym, common.t_, prj_trans, tr_,
                     feature, common.vars_, common.scale_factor_)
    {
        value_double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym_, feature_, common_.vars_);
        value_double smooth = get<value_double, keys::smooth>(sym_, feature_, common_.vars_);

        if (simplify_tolerance > 0.0) converter_.template set<simplify_tag>();
        converter_.template set<affine_transform_tag>();
        if (smooth > 0.0) converter_.template set<smooth_tag>();
    }

    void render(renderer_base & ren_base, rasterizer & ras)
    {
        coord<double, 2> offset(pattern_offset(sym_, feature_, prj_trans_, common_,
                                               pattern_img_.width(), pattern_img_.height()));
        agg::rendering_buffer pattern_rbuf((agg::int8u*)pattern_img_.bytes(),
                                           pattern_img_.width(), pattern_img_.height(),
                                           pattern_img_.width() * 4);
        agg::pixfmt_rgba32_pre pixf_pattern(pattern_rbuf);
        img_source_type img_src(pixf_pattern);
        span_gen_type sg(img_src, safe_cast<unsigned>(offset.x), safe_cast<unsigned>(offset.y));

        agg::span_allocator<agg::rgba8> sa;
        value_double opacity = get<double, keys::opacity>(sym_, feature_, common_.vars_);
        renderer_type rp(ren_base, sa, sg, unsigned(opacity * 255));

        using apply_vertex_converter_type = detail::apply_vertex_converter<
            VertexConverter, rasterizer>;
        using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
        apply_vertex_converter_type apply(converter_, ras);
        mapnik::util::apply_visitor(vertex_processor_type(apply), feature_.get_geometry());
        agg::scanline_u8 sl;
        agg::render_scanlines(ras, sl, rp);
    }

    const bool clip_;
    const box2d<double> clip_box_;
    const agg::trans_affine tr_;
    VertexConverter converter_;
};

} // namespace mapnik


#endif // MAPNIK_RENDER_POLYGON_PATTERN_HPP
