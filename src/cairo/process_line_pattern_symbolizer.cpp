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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/cairo/render_polygon_pattern.hpp>
#include <mapnik/renderer_common/render_pattern.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>

#include <memory>

namespace mapnik {

namespace {

struct prepare_pattern_visitor
{
    prepare_pattern_visitor(renderer_common const& common,
                            symbolizer_base const& sym,
                            feature_impl const& feature,
                            std::size_t& width,
                            std::size_t& height)
        : common_(common)
        , sym_(sym)
        , feature_(feature)
        , width_(width)
        , height_(height)
    {}

    std::shared_ptr<cairo_pattern> operator()(mapnik::marker_null const&)
    {
        throw std::runtime_error("This should not have been reached.");
    }

    std::shared_ptr<cairo_pattern> operator()(mapnik::marker_svg const& marker)
    {
        double opacity = get<value_double, keys::opacity>(sym_, feature_, common_.vars_);
        agg::trans_affine image_tr = agg::trans_affine_scaling(common_.scale_factor_);
        auto image_transform = get_optional<transform_type>(sym_, keys::image_transform);
        if (image_transform)
            evaluate_transform(image_tr, feature_, common_.vars_, *image_transform, common_.scale_factor_);
        mapnik::box2d<double> const& bbox_image = marker.get_data()->bounding_box() * image_tr;
        mapnik::image_rgba8 image(bbox_image.width(), bbox_image.height());
        render_pattern<image_rgba8>(marker, image_tr, 1.0, image);
        width_ = image.width();
        height_ = image.height();
        return std::make_shared<cairo_pattern>(image, opacity);
    }

    std::shared_ptr<cairo_pattern> operator()(mapnik::marker_rgba8 const& marker)
    {
        double opacity = get<value_double, keys::opacity>(sym_, feature_, common_.vars_);
        return std::make_shared<cairo_pattern>(marker.get_data(), opacity);
    }

  private:
    renderer_common const& common_;
    symbolizer_base const& sym_;
    feature_impl const& feature_;
    std::size_t& width_;
    std::size_t& height_;
};

template<typename... Converters>
using vertex_converter_type = vertex_converter<clip_line_tag,
                                               transform_tag,
                                               affine_transform_tag,
                                               simplify_tag,
                                               smooth_tag,
                                               offset_transform_tag,
                                               Converters...>;

struct warp_pattern : cairo_pattern_base
{
    using vc_type = vertex_converter_type<>;

    warp_pattern(mapnik::marker const& marker,
                 renderer_common const& common,
                 symbolizer_base const& sym,
                 mapnik::feature_impl const& feature,
                 proj_transform const& prj_trans)
        : cairo_pattern_base{marker, common, sym, feature, prj_trans}
        , clip_(get<value_bool, keys::clip>(sym, feature, common.vars_))
        , offset_(get<value_double, keys::offset>(sym, feature, common.vars_))
        , clip_box_(clipping_extent(common))
        , tr_(geom_transform())
        , converter_(clip_box_, sym, common.t_, prj_trans, tr_, feature, common.vars_, common.scale_factor_)
    {
        value_double offset = get<value_double, keys::offset>(sym, feature, common.vars_);
        value_double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym, feature, common.vars_);
        value_double smooth = get<value_double, keys::smooth>(sym, feature, common.vars_);

        if (std::fabs(offset) > 0.0)
            converter_.template set<offset_transform_tag>();
        converter_.template set<affine_transform_tag>();
        if (simplify_tolerance > 0.0)
            converter_.template set<simplify_tag>();
        if (smooth > 0.0)
            converter_.template set<smooth_tag>();
        if (clip_)
            converter_.template set<clip_line_tag>();
    }

    box2d<double> clip_box() const
    {
        box2d<double> clipping_extent = common_.query_extent_;
        if (clip_)
        {
            double pad_per_pixel = static_cast<double>(common_.query_extent_.width() / common_.width_);
            double pixels = std::ceil(std::max(marker_.width() / 2.0 + std::fabs(offset_),
                                               (std::fabs(offset_) * offset_converter_default_threshold)));
            double padding = pad_per_pixel * pixels * common_.scale_factor_;

            clipping_extent.pad(padding);
        }
        return clipping_extent;
    }

    void render(cairo_context& context)
    {
        std::size_t width = marker_.width();
        std::size_t height = marker_.height();

        // TODO - re-implement at renderer level like polygon_pattern symbolizer
        prepare_pattern_visitor visit(common_, sym_, feature_, width, height);
        std::shared_ptr<cairo_pattern> pattern = util::apply_visitor(visit, marker_);
        pattern->set_extend(CAIRO_EXTEND_REPEAT);
        pattern->set_filter(CAIRO_FILTER_BILINEAR);

        composite_mode_e comp_op = get<composite_mode_e, keys::comp_op>(sym_, feature_, common_.vars_);

        cairo_save_restore guard(context);
        context.set_operator(comp_op);
        context.set_line_width(height);

        using rasterizer_type = line_pattern_rasterizer<cairo_context>;
        rasterizer_type ras(context, *pattern, width, height);

        using apply_vertex_converter_type = detail::apply_vertex_converter<vc_type, rasterizer_type>;
        using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
        apply_vertex_converter_type apply(converter_, ras);
        mapnik::util::apply_visitor(vertex_processor_type(apply), feature_.get_geometry());
    }

    const bool clip_;
    const double offset_;
    const box2d<double> clip_box_;
    const agg::trans_affine tr_;
    vc_type converter_;
};

using repeat_pattern_base = cairo_polygon_pattern<vertex_converter_type<dash_tag, stroke_tag>>;

struct repeat_pattern : repeat_pattern_base
{
    using repeat_pattern_base::cairo_polygon_pattern;

    void render(cairo_context& context)
    {
        if (has_key(sym_, keys::stroke_dasharray))
        {
            converter_.template set<dash_tag>();
        }

        if (clip_)
            converter_.template set<clip_line_tag>();

        value_double offset = get<value_double, keys::offset>(sym_, feature_, common_.vars_);
        if (std::fabs(offset) > 0.0)
            converter_.template set<offset_transform_tag>();

        repeat_pattern_base::render(CAIRO_FILL_RULE_WINDING, context);
    }
};

} // namespace

template<typename T>
void cairo_renderer<T>::process(line_pattern_symbolizer const& sym,
                                feature_impl& feature,
                                proj_transform const& prj_trans)
{
    std::string filename = get<std::string, keys::file>(sym, feature, common_.vars_);
    std::shared_ptr<mapnik::marker const> marker = marker_cache::instance().find(filename, true);

    if (marker->is<mapnik::marker_null>())
    {
        return;
    }

    const line_pattern_enum pattern = get<line_pattern_enum, keys::line_pattern>(sym, feature, common_.vars_);
    switch (pattern)
    {
        case line_pattern_enum::LINE_PATTERN_WARP: {
            warp_pattern pattern(*marker, common_, sym, feature, prj_trans);
            pattern.render(context_);
            break;
        }
        case line_pattern_enum::LINE_PATTERN_REPEAT: {
            repeat_pattern pattern(*marker, common_, sym, feature, prj_trans);
            pattern.render(context_);
            break;
        }
        case line_pattern_enum::line_pattern_enum_MAX:
        default:
            MAPNIK_LOG_ERROR(process_line_pattern_symbolizer) << "Incorrect line-pattern value.";
    }
}

template void
  cairo_renderer<cairo_ptr>::process(line_pattern_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik

#endif // HAVE_CAIRO
