/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/make_unique.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/renderer_common/render_pattern.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>

namespace mapnik
{

struct cairo_renderer_process_visitor_l
{
    cairo_renderer_process_visitor_l(renderer_common const& common,
                                   line_pattern_symbolizer const& sym,
                                   mapnik::feature_impl & feature,
                                   std::size_t & width,
                                   std::size_t & height)
        : common_(common),
          sym_(sym),
          feature_(feature),
          width_(width),
          height_(height) {}

    std::shared_ptr<cairo_pattern> operator() (mapnik::marker_null const&)
    {
        throw std::runtime_error("This should not have been reached.");
    }

    std::shared_ptr<cairo_pattern> operator() (mapnik::marker_svg const& marker)
    {
        double opacity = get<value_double, keys::opacity>(sym_, feature_, common_.vars_);
        mapnik::rasterizer ras;
        agg::trans_affine image_tr = agg::trans_affine_scaling(common_.scale_factor_);
        auto image_transform = get_optional<transform_type>(sym_, keys::image_transform);
        if (image_transform) evaluate_transform(image_tr, feature_, common_.vars_, *image_transform);
        mapnik::box2d<double> const& bbox_image = marker.get_data()->bounding_box() * image_tr;
        mapnik::image_rgba8 image(bbox_image.width(), bbox_image.height());
        render_pattern<image_rgba8>(ras, marker, image_tr, 1.0, image);
        width_ = image.width();
        height_ = image.height();
        return std::make_shared<cairo_pattern>(image, opacity);
    }

    std::shared_ptr<cairo_pattern> operator() (mapnik::marker_rgba8 const& marker)
    {
        double opacity = get<value_double, keys::opacity>(sym_, feature_, common_.vars_);
        return std::make_shared<cairo_pattern>(marker.get_data(), opacity);
    }

  private:
    renderer_common const& common_;
    line_pattern_symbolizer const& sym_;
    mapnik::feature_impl & feature_;
    std::size_t & width_;
    std::size_t & height_;
};

template <typename T>
void cairo_renderer<T>::process(line_pattern_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    std::string filename = get<std::string, keys::file>(sym, feature, common_.vars_);
    composite_mode_e comp_op = get<composite_mode_e, keys::comp_op>(sym, feature, common_.vars_);
    value_bool clip = get<value_bool, keys::clip>(sym, feature, common_.vars_);
    value_double offset = get<value_double, keys::offset>(sym, feature, common_.vars_);
    value_double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym, feature, common_.vars_);
    value_double smooth = get<value_double, keys::smooth>(sym, feature, common_.vars_);

    if (filename.empty())
    {
        return;
    }

    std::shared_ptr<mapnik::marker const> marker = marker_cache::instance().find(filename, true);

    if (marker->is<mapnik::marker_null>()) return;

    std::size_t width = marker->width();
    std::size_t height = marker->height();

    cairo_save_restore guard(context_);
    context_.set_operator(comp_op);
    // TODO - re-implement at renderer level like polygon_pattern symbolizer
    cairo_renderer_process_visitor_l visit(common_,
                                           sym,
                                           feature,
                                           width,
                                           height);
    std::shared_ptr<cairo_pattern> pattern = util::apply_visitor(visit, *marker);

    context_.set_line_width(height);

    pattern->set_extend(CAIRO_EXTEND_REPEAT);
    pattern->set_filter(CAIRO_FILTER_BILINEAR);

    agg::trans_affine tr;
    auto geom_transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (geom_transform) { evaluate_transform(tr, feature, common_.vars_, *geom_transform, common_.scale_factor_); }

    box2d<double> clipping_extent = common_.query_extent_;
    if (clip)
    {
        double padding = (double)(common_.query_extent_.width()/common_.width_);
        double half_stroke = width/2.0;
        if (half_stroke > 1)
            padding *= half_stroke;
        if (std::fabs(offset) > 0)
            padding *= std::fabs(offset) * 1.2;
        padding *= common_.scale_factor_;
        clipping_extent.pad(padding);
    }

    using rasterizer_type = line_pattern_rasterizer<cairo_context>;
    rasterizer_type ras(context_, *pattern, width, height);
    using vertex_converter_type = vertex_converter<clip_line_tag, transform_tag,
                                                   affine_transform_tag,
                                                   simplify_tag, smooth_tag,
                                                   offset_transform_tag>;

    vertex_converter_type converter(clipping_extent,sym, common_.t_, prj_trans, tr, feature, common_.vars_, common_.scale_factor_);

    if (clip) converter.set<clip_line_tag>();
    converter.set<transform_tag>(); // always transform
    if (std::fabs(offset) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
    converter.set<affine_transform_tag>(); // optional affine transform
    if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter

    using apply_vertex_converter_type = detail::apply_vertex_converter<vertex_converter_type, rasterizer_type>;
    using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
    apply_vertex_converter_type apply(converter, ras);
    mapnik::util::apply_visitor(vertex_processor_type(apply), feature.get_geometry());
}

template void cairo_renderer<cairo_ptr>::process(line_pattern_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
