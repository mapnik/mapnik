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
#include <mapnik/feature.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/image_any.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/agg_pattern_source.hpp>
#include <mapnik/agg/render_polygon_pattern.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>
#include <mapnik/renderer_common/render_pattern.hpp>
#include <mapnik/renderer_common/pattern_alignment.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_outline_aa.h"
#include "agg_pattern_filters_rgba.h"
#include "agg_renderer_outline_image.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

namespace {

template<typename... Converters>
using vertex_converter_type = vertex_converter<clip_line_tag,
                                               transform_tag,
                                               affine_transform_tag,
                                               simplify_tag,
                                               smooth_tag,
                                               offset_transform_tag,
                                               Converters...>;

struct warp_pattern : agg_pattern_base
{
    using vc_type = vertex_converter_type<>;
    using color_type = agg::rgba8;
    using order_type = agg::order_rgba;
    using blender_type = agg::comp_op_adaptor_rgba_pre<color_type, order_type>;
    using pattern_filter_type = agg::pattern_filter_bilinear_rgba8;
    using pattern_type = agg::line_image_pattern<pattern_filter_type>;
    using pixfmt_type = agg::pixfmt_custom_blend_rgba<blender_type, agg::rendering_buffer>;
    using renderer_base = agg::renderer_base<pixfmt_type>;
    using renderer_type = agg::renderer_outline_image<renderer_base, pattern_type>;
    using rasterizer_type = agg::rasterizer_outline_aa<renderer_type>;

    warp_pattern(image_rgba8 const& pattern_img,
                 renderer_common const& common,
                 symbolizer_base const& sym,
                 mapnik::feature_impl const& feature,
                 proj_transform const& prj_trans)
        : agg_pattern_base{pattern_img, common, sym, feature, prj_trans}
        , clip_(get<value_bool, keys::clip>(sym, feature, common.vars_))
        , offset_(get<value_double, keys::offset>(sym, feature, common.vars_))
        , clip_box_(clip_box())
        , tr_(geom_transform())
        , converter_(clip_box_, sym, common.t_, prj_trans, tr_, feature, common.vars_, common.scale_factor_)
    {
        value_double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym_, feature_, common_.vars_);
        value_double smooth = get<value_double, keys::smooth>(sym_, feature_, common_.vars_);

        if (std::fabs(offset_) > 0.0)
            converter_.template set<offset_transform_tag>();
        if (simplify_tolerance > 0.0)
            converter_.template set<simplify_tag>();
        converter_.template set<affine_transform_tag>();
        if (smooth > 0.0)
            converter_.template set<smooth_tag>();
    }

    box2d<double> clip_box() const
    {
        box2d<double> clip_box = clipping_extent(common_);
        if (clip_)
        {
            double pad_per_pixel = static_cast<double>(common_.query_extent_.width() / common_.width_);
            double pixels = std::ceil(std::max(pattern_img_.width() / 2.0 + std::fabs(offset_),
                                               (std::fabs(offset_) * offset_converter_default_threshold)));
            double padding = pad_per_pixel * pixels * common_.scale_factor_;
            clip_box.pad(padding);
        }
        return clip_box;
    }

    void render(renderer_base& ren_base, rasterizer&)
    {
        value_double opacity = get<double, keys::opacity>(sym_, feature_, common_.vars_);
        agg::pattern_filter_bilinear_rgba8 filter;
        pattern_source source(pattern_img_, opacity);
        pattern_type pattern(filter, source);
        renderer_type ren(ren_base, pattern);
        double half_stroke = std::max(pattern_img_.width() / 2.0, pattern_img_.height() / 2.0);
        int rast_clip_padding = static_cast<int>(std::round(half_stroke));
        ren.clip_box(-rast_clip_padding,
                     -rast_clip_padding,
                     common_.width_ + rast_clip_padding,
                     common_.height_ + rast_clip_padding);
        rasterizer_type ras(ren);

        using apply_vertex_converter_type = detail::apply_vertex_converter<vc_type, rasterizer_type>;
        using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
        apply_vertex_converter_type apply(converter_, ras);

        util::apply_visitor(vertex_processor_type(apply), feature_.get_geometry());
    }

    const bool clip_;
    const double offset_;
    const box2d<double> clip_box_;
    const agg::trans_affine tr_;
    vc_type converter_;
};

using repeat_pattern_base = agg_polygon_pattern<vertex_converter_type<dash_tag, stroke_tag>>;
struct repeat_pattern : repeat_pattern_base
{
    using repeat_pattern_base::agg_polygon_pattern;

    void render(renderer_base& ren_base, rasterizer& ras)
    {
        if (has_key(sym_, keys::stroke_dasharray))
        {
            converter_.set<dash_tag>();
        }

        if (clip_)
            converter_.template set<clip_line_tag>();

        value_double offset = get<value_double, keys::offset>(sym_, feature_, common_.vars_);
        if (std::fabs(offset) > 0.0)
            converter_.template set<offset_transform_tag>();

        // To allow lines cross themselves.
        ras.filling_rule(agg::fill_non_zero);

        repeat_pattern_base::render(ren_base, ras);
    }
};

} // namespace

template<typename buffer_type>
struct agg_renderer_process_visitor_l
{
    agg_renderer_process_visitor_l(renderer_common const& common,
                                   buffer_type& current_buffer,
                                   rasterizer& ras,
                                   line_pattern_symbolizer const& sym,
                                   mapnik::feature_impl const& feature,
                                   proj_transform const& prj_trans)
        : common_(common)
        , current_buffer_(current_buffer)
        , ras_(ras)
        , sym_(sym)
        , feature_(feature)
        , prj_trans_(prj_trans)
    {}

    void operator()(marker_null const&) const {}

    void operator()(marker_svg const& marker) const
    {
        agg::trans_affine image_tr = agg::trans_affine_scaling(common_.scale_factor_);
        auto image_transform = get_optional<transform_type>(sym_, keys::image_transform);
        if (image_transform)
            evaluate_transform(image_tr, feature_, common_.vars_, *image_transform, common_.scale_factor_);
        mapnik::box2d<double> const& bbox_image = marker.get_data()->bounding_box() * image_tr;
        image_rgba8 image(bbox_image.width(), bbox_image.height());
        render_pattern<buffer_type>(marker, image_tr, 1.0, image);
        render_by_pattern_type(image);
    }

    void operator()(marker_rgba8 const& marker) const { render_by_pattern_type(marker.get_data()); }

  private:
    void render_by_pattern_type(image_rgba8 const& pattern_image) const
    {
        line_pattern_enum pattern = get<line_pattern_enum, keys::line_pattern>(sym_, feature_, common_.vars_);
        switch (pattern)
        {
            case line_pattern_enum::LINE_PATTERN_WARP: {
                warp_pattern pattern(pattern_image, common_, sym_, feature_, prj_trans_);
                render(pattern);
                break;
            }
            case line_pattern_enum::LINE_PATTERN_REPEAT: {
                repeat_pattern pattern(pattern_image, common_, sym_, feature_, prj_trans_);
                render(pattern);
                break;
            }
            case line_pattern_enum::line_pattern_enum_MAX:
            default:
                MAPNIK_LOG_ERROR(process_line_pattern_symbolizer) << "Incorrect line-pattern value.";
        }
    }

    template<typename Pattern>
    void render(Pattern& pattern) const
    {
        agg::rendering_buffer buf(current_buffer_.bytes(),
                                  current_buffer_.width(),
                                  current_buffer_.height(),
                                  current_buffer_.row_size());
        typename Pattern::pixfmt_type pixf(buf);
        pixf.comp_op(static_cast<agg::comp_op_e>(get<composite_mode_e, keys::comp_op>(sym_, feature_, common_.vars_)));
        typename Pattern::renderer_base ren_base(pixf);

        if (pattern.clip_)
            pattern.converter_.template set<clip_line_tag>();
        pattern.render(ren_base, ras_);
    }

    renderer_common const& common_;
    buffer_type& current_buffer_;
    rasterizer& ras_;
    line_pattern_symbolizer const& sym_;
    mapnik::feature_impl const& feature_;
    proj_transform const& prj_trans_;
};

template<typename T0, typename T1>
void agg_renderer<T0, T1>::process(line_pattern_symbolizer const& sym,
                                   mapnik::feature_impl& feature,
                                   proj_transform const& prj_trans)
{
    std::string filename = get<std::string, keys::file>(sym, feature, common_.vars_);
    if (filename.empty())
        return;
    ras_ptr->reset();
    if (gamma_method_ != gamma_method_enum::GAMMA_POWER || gamma_ != 1.0)
    {
        ras_ptr->gamma(agg::gamma_power());
        gamma_method_ = gamma_method_enum::GAMMA_POWER;
        gamma_ = 1.0;
    }

    std::shared_ptr<mapnik::marker const> marker = marker_cache::instance().find(filename, true);
    agg_renderer_process_visitor_l<buffer_type> visitor(common_,
                                                        buffers_.top().get(),
                                                        *ras_ptr,
                                                        sym,
                                                        feature,
                                                        prj_trans);
    util::apply_visitor(visitor, *marker);
}

template void
  agg_renderer<image_rgba8>::process(line_pattern_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik
