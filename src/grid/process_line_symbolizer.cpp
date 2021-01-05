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

#if defined(GRID_RENDERER)

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>
#include <mapnik/geometry/geometry_type.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_bin.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
MAPNIK_DISABLE_WARNING_POP

// stl
#include <string>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(line_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    using pixfmt_type = typename grid_renderer_base_type::pixfmt_type;
    using color_type = typename grid_renderer_base_type::pixfmt_type::color_type;
    using renderer_type = agg::renderer_scanline_bin_solid<grid_renderer_base_type>;

    agg::scanline_bin sl;

    grid_rendering_buffer buf(pixmap_.raw_data(), common_.width_, common_.height_, common_.width_);
    pixfmt_type pixf(buf);

    grid_renderer_base_type renb(pixf);
    renderer_type ren(renb);

    ras_ptr->reset();

    agg::trans_affine tr;
    auto transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (transform)
    {
        evaluate_transform(tr, feature, common_.vars_, *transform, common_.scale_factor_);
    }

    box2d<double> clipping_extent = common_.query_extent_;

    bool clip = get<value_bool>(sym, keys::clip, feature, common_.vars_, false);
    double width = get<value_double>(sym, keys::stroke_width, feature, common_.vars_,1.0);
    double offset = get<value_double>(sym, keys::offset, feature, common_.vars_,0.0);
    double simplify_tolerance = get<value_double>(sym, keys::simplify_tolerance, feature, common_.vars_,0.0);
    double smooth = get<value_double>(sym, keys::smooth, feature, common_.vars_,false);
    bool has_dash = has_key(sym, keys::stroke_dasharray);

    if (clip)
    {
        double pad_per_pixel = static_cast<double>(common_.query_extent_.width()/common_.width_);
        double pixels = std::ceil(std::max(width / 2.0 + std::fabs(offset),
                                          (std::fabs(offset) * offset_converter_default_threshold)));
        double padding = pad_per_pixel * pixels * common_.scale_factor_;

        clipping_extent.pad(padding);
    }
    using vertex_converter_type = vertex_converter<clip_line_tag, clip_poly_tag, transform_tag,
                                                   affine_transform_tag,
                                                   simplify_tag, smooth_tag,
                                                   offset_transform_tag,
                                                   dash_tag, stroke_tag>;

    vertex_converter_type converter(clipping_extent,sym,common_.t_,prj_trans,tr,feature,common_.vars_,common_.scale_factor_);
    if (clip)
    {
        geometry::geometry_types type = geometry::geometry_type(feature.get_geometry());
        if (type == geometry::geometry_types::Polygon || type == geometry::geometry_types::MultiPolygon)
            converter.template set<clip_poly_tag>();
        else if (type == geometry::geometry_types::LineString || type == geometry::geometry_types::MultiLineString)
            converter.template set<clip_line_tag>();
    }
    converter.set<transform_tag>(); // always transform
    if (std::fabs(offset) > 0.0) converter.set<offset_transform_tag>(); // parallel offset
    converter.set<affine_transform_tag>(); // optional affine transform
    if (simplify_tolerance > 0.0) converter.set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter.set<smooth_tag>(); // optional smooth converter
    if (has_dash) converter.set<dash_tag>();
    converter.set<stroke_tag>(); //always stroke

    using apply_vertex_converter_type = detail::apply_vertex_converter<vertex_converter_type, grid_rasterizer>;
    using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
    apply_vertex_converter_type apply(converter, *ras_ptr);
    mapnik::util::apply_visitor(vertex_processor_type(apply),feature.get_geometry());

    // render id
    ren.color(color_type(feature.id()));
    ras_ptr->filling_rule(agg::fill_non_zero);
    agg::render_scanlines(*ras_ptr, sl, ren);

    // add feature properties to grid cache
    pixmap_.add_feature(feature);

}


template void grid_renderer<grid>::process(line_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}

#endif
