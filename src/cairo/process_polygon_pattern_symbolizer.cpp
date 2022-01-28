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
#include <mapnik/cairo/cairo_render_vector.hpp>
#include <mapnik/cairo/render_polygon_pattern.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>
#include <mapnik/renderer_common/pattern_alignment.hpp>

namespace mapnik {

template<typename T>
void cairo_renderer<T>::process(polygon_pattern_symbolizer const& sym,
                                mapnik::feature_impl& feature,
                                proj_transform const& prj_trans)
{
    std::string filename = get<std::string, keys::file>(sym, feature, common_.vars_);
    std::shared_ptr<mapnik::marker const> marker = mapnik::marker_cache::instance().find(filename, true);
    if (marker->is<mapnik::marker_null>())
        return;

    using vertex_converter_type =
      vertex_converter<clip_poly_tag, transform_tag, affine_transform_tag, simplify_tag, smooth_tag>;
    using pattern_type = cairo_polygon_pattern<vertex_converter_type>;

    pattern_type pattern(*marker, common_, sym, feature, prj_trans);

    if (prj_trans.equal() && pattern.clip_)
        pattern.converter_.set<clip_poly_tag>();

    pattern.render(CAIRO_FILL_RULE_EVEN_ODD, context_);
}

template void
  cairo_renderer<cairo_ptr>::process(polygon_pattern_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik

#endif // HAVE_CAIRO
