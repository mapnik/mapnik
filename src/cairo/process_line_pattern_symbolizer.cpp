/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/agg_rasterizer.hpp>

namespace mapnik
{

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

    boost::optional<marker_ptr> marker;
    if ( !filename.empty() )
    {
        marker = marker_cache::instance().find(filename, true);
    }
    if (!marker || !(*marker)) return;

    unsigned width = (*marker)->width();
    unsigned height = (*marker)->height();

    cairo_save_restore guard(context_);
    context_.set_operator(comp_op);
    std::shared_ptr<cairo_pattern> pattern;
    image_ptr image = nullptr;
    // TODO - re-implement at renderer level like polygon_pattern symbolizer
    double opacity = get<value_double, keys::opacity>(sym, feature, common_.vars_);
    if ((*marker)->is_bitmap())
    {
        pattern = std::make_unique<cairo_pattern>(**((*marker)->get_bitmap_data()), opacity);
        context_.set_line_width(height);
    }
    else
    {
        mapnik::rasterizer ras;
        agg::trans_affine image_tr = agg::trans_affine_scaling(common_.scale_factor_);
        auto image_transform = get_optional<transform_type>(sym, keys::image_transform);
        if (image_transform) evaluate_transform(image_tr, feature, common_.vars_, *image_transform);
        image = render_pattern(ras, **marker, image_tr, 1.0);
        pattern = std::make_unique<cairo_pattern>(*image, opacity);
        width = image->width();
        height = image->height();
        context_.set_line_width(height);
    }

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
    vertex_converter<rasterizer_type,clip_line_tag, transform_tag,
                     affine_transform_tag,
                     simplify_tag, smooth_tag,
                     offset_transform_tag,
                     dash_tag, stroke_tag>
        converter(clipping_extent, ras, sym, common_.t_, prj_trans, tr, feature, common_.vars_, common_.scale_factor_);

    if (clip) converter.set<clip_line_tag>(); // optional clip (default: true)
    converter.set<transform_tag>(); // always transform
    if (std::fabs(offset) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
    converter.set<affine_transform_tag>(); // optional affine transform
    if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter


    for (auto & geom : feature.paths())
    {
        if (geom.size() > 1)
        {
            converter.apply(geom);
        }
    }
}

template void cairo_renderer<cairo_ptr>::process(line_pattern_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
