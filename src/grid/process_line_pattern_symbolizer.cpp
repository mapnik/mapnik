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

#if defined(GRID_RENDERER)

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rasterizer_scanline_aa.h"
#include "agg_renderer_scanline.h"
#include "agg_scanline_bin.h"
MAPNIK_DISABLE_WARNING_POP

// stl
#include <string>

namespace mapnik {

template<typename T>
void grid_renderer<T>::process(line_pattern_symbolizer const& sym,
                               mapnik::feature_impl& feature,
                               proj_transform const& prj_trans)
{
    const std::string filename = get<std::string, keys::file>(sym, feature, common_.vars_);
    if (filename.empty())
        return;
    const auto mark = marker_cache::instance().find(filename, true);
    if (mark->is<mapnik::marker_null>())
        return;

    if (!mark->is<mapnik::marker_rgba8>())
    {
        MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: Only images (not '" << filename
                                       << "') are supported in the line_pattern_symbolizer";
        return;
    }

    const value_bool clip = get<value_bool, keys::clip>(sym, feature, common_.vars_);
    const value_double offset = get<value_double, keys::offset>(sym, feature, common_.vars_);
    const value_double simplify_tolerance = get<value_double, keys::simplify_tolerance>(sym, feature, common_.vars_);
    const value_double smooth = get<value_double, keys::smooth>(sym, feature, common_.vars_);

    using pixfmt_type = typename grid_renderer_base_type::pixfmt_type;
    using color_type = typename grid_renderer_base_type::pixfmt_type::color_type;
    using renderer_type = agg::renderer_scanline_bin_solid<grid_renderer_base_type>;

    agg::scanline_bin sl;

    grid_rendering_buffer buf(pixmap_.raw_data(), common_.width_, common_.height_, common_.width_);
    pixfmt_type pixf(buf);

    grid_renderer_base_type renb(pixf);
    renderer_type ren(renb);

    ras_ptr->reset();

    line_pattern_enum pattern = get<line_pattern_enum, keys::line_pattern>(sym, feature, common_.vars_);
    const std::size_t stroke_width = (pattern == line_pattern_enum::LINE_PATTERN_WARP)
                                       ? mark->width()
                                       : get<value_double, keys::stroke_width>(sym, feature, common_.vars_);

    agg::trans_affine tr;
    const auto transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (transform)
    {
        evaluate_transform(tr, feature, common_.vars_, *transform, common_.scale_factor_);
    }

    box2d<double> clipping_extent = common_.query_extent_;
    if (clip)
    {
        const double pad_per_pixel = static_cast<double>(common_.query_extent_.width() / common_.width_);
        const double pixels = std::ceil(
          std::max(stroke_width / 2.0 + std::fabs(offset), (std::fabs(offset) * offset_converter_default_threshold)));
        const double padding = pad_per_pixel * pixels * common_.scale_factor_;

        clipping_extent.pad(padding);
    }

    // to avoid the complexity of using an agg pattern filter instead
    // we create a line_symbolizer in order to fake the pattern
    line_symbolizer line;
    put<value_double>(line, keys::stroke_width, value_double(stroke_width));
    // TODO: really should pass the offset to the fake line too, but
    // this wasn't present in the previous version and makes the test
    // fail - in this case, probably the test should be updated.
    // put<value_double>(line, keys::offset, value_double(offset));
    put<value_double>(line, keys::simplify_tolerance, value_double(simplify_tolerance));
    put<value_double>(line, keys::smooth, value_double(smooth));

    using vertex_converter_type = vertex_converter<clip_line_tag,
                                                   transform_tag,
                                                   affine_transform_tag,
                                                   simplify_tag,
                                                   smooth_tag,
                                                   offset_transform_tag,
                                                   stroke_tag>;
    vertex_converter_type
      converter(clipping_extent, line, common_.t_, prj_trans, tr, feature, common_.vars_, common_.scale_factor_);
    if (clip)
        converter.set<clip_line_tag>();
    converter.set<transform_tag>(); // always transform
    if (std::fabs(offset) > 0.0)
        converter.set<offset_transform_tag>(); // parallel offset
    converter.set<affine_transform_tag>();     // optional affine transform
    if (simplify_tolerance > 0.0)
        converter.set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0)
        converter.set<smooth_tag>(); // optional smooth converter
    converter.set<stroke_tag>();     // always stroke
    using apply_vertex_converter_type = detail::apply_vertex_converter<vertex_converter_type, grid_rasterizer>;
    using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;
    apply_vertex_converter_type apply(converter, *ras_ptr);
    mapnik::util::apply_visitor(vertex_processor_type(apply), feature.get_geometry());

    // render id
    ren.color(color_type(feature.id()));
    agg::render_scanlines(*ras_ptr, sl, ren);

    // add feature properties to grid cache
    pixmap_.add_feature(feature);
}

template void
  grid_renderer<grid>::process(line_pattern_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik

#endif
