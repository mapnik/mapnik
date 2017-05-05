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

#ifndef MAPNIK_RENDERER_COMMON_PROCESS_POLYGON_SYMBOLIZER_HPP
#define MAPNIK_RENDERER_COMMON_PROCESS_POLYGON_SYMBOLIZER_HPP

#include <mapnik/renderer_common.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/symbolizer.hpp>

#include <mapnik/feature.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>

namespace mapnik {

template <typename vertex_converter_type, typename rasterizer_type, typename F>
void render_polygon_symbolizer(polygon_symbolizer const &sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans,
                               renderer_common & common,
                               box2d<double> const& clip_box,
                               rasterizer_type & ras,
                               F fill_func)
{
    agg::trans_affine tr;
    auto transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (transform) evaluate_transform(tr, feature, common.vars_, *transform, common.scale_factor_);

    value_bool clip = get<value_bool,keys::clip>(sym, feature, common.vars_);
    value_double simplify_tolerance = get<value_double,keys::simplify_tolerance>(sym, feature, common.vars_);
    value_double smooth = get<value_double,keys::smooth>(sym, feature, common.vars_);
    value_double opacity = get<value_double,keys::fill_opacity>(sym, feature, common.vars_);

    vertex_converter_type converter(clip_box, sym, common.t_, prj_trans, tr,
                                    feature,common.vars_,common.scale_factor_);

    if (prj_trans.equal() && clip) converter.template set<clip_poly_tag>();
    converter.template set<transform_tag>(); //always transform
    converter.template set<affine_transform_tag>();
    if (simplify_tolerance > 0.0) converter.template set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter.template set<smooth_tag>(); // optional smooth converter

    using apply_vertex_converter_type = detail::apply_vertex_converter<vertex_converter_type, rasterizer_type>;
    using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
    apply_vertex_converter_type apply(converter, ras);
    mapnik::util::apply_visitor(vertex_processor_type(apply),feature.get_geometry());

    color const& fill = get<mapnik::color, keys::fill>(sym, feature, common.vars_);
    fill_func(fill, opacity);
}

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_PROCESS_POLYGON_SYMBOLIZER_HPP
