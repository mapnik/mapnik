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
#include <mapnik/feature.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>

namespace mapnik
{

template <typename T>
void cairo_renderer<T>::process(line_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    composite_mode_e comp_op = get<composite_mode_e, keys::comp_op>(sym, feature, common_.vars_);
    value_bool clip = get<value_bool, keys::clip>(sym, feature, common_.vars_);
    value_double offset = get<value_double, keys::offset>(sym, feature, common_.vars_);
    value_double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym, feature, common_.vars_);
    value_double smooth = get<value_double, keys::smooth>(sym, feature, common_.vars_);

    color stroke = get<color, keys::stroke>(sym, feature, common_.vars_);
    value_double stroke_opacity = get<value_double, keys::stroke_opacity>(sym, feature, common_.vars_);
    line_join_enum stroke_join = get<line_join_enum, keys::stroke_linejoin>(sym, feature, common_.vars_);
    line_cap_enum stroke_cap = get<line_cap_enum, keys::stroke_linecap>(sym, feature, common_.vars_);
    value_double miterlimit = get<value_double, keys::stroke_miterlimit>(sym, feature, common_.vars_);
    value_double width = get<value_double, keys::stroke_width>(sym, feature, common_.vars_);

    auto dash = get_optional<dash_array>(sym, keys::stroke_dasharray, feature, common_.vars_);

    cairo_save_restore guard(context_);
    context_.set_operator(comp_op);
    context_.set_color(stroke, stroke_opacity);
    context_.set_line_join(stroke_join);
    context_.set_line_cap(stroke_cap);
    context_.set_miter_limit(miterlimit);
    context_.set_line_width(width * common_.scale_factor_);
    if (dash)
    {
        context_.set_dash(*dash, common_.scale_factor_);
    }

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
    using vertex_converter_type =  vertex_converter<clip_line_tag,
                                                    transform_tag,
                                                    affine_transform_tag,
                                                    simplify_tag, smooth_tag,
                                                    offset_transform_tag>;

    vertex_converter_type converter(clipping_extent,sym,common_.t_,prj_trans,tr,feature,common_.vars_,common_.scale_factor_);

    if (clip) converter.set<clip_line_tag>(); // optional clip (default: true)
    converter.set<transform_tag>(); // always transform
    if (std::fabs(offset) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
    converter.set<affine_transform_tag>(); // optional affine transform
    if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter
    using apply_vertex_converter_type = detail::apply_vertex_converter<vertex_converter_type, cairo_context>;
    using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
    apply_vertex_converter_type apply(converter, context_);
    mapnik::util::apply_visitor(vertex_processor_type(apply),feature.get_geometry());
    // stroke
    context_.set_fill_rule(CAIRO_FILL_RULE_WINDING);
    context_.stroke();
}

template void cairo_renderer<cairo_ptr>::process(line_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
