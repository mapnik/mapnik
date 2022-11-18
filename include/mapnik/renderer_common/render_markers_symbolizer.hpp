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

#ifndef MAPNIK_RENDERER_COMMON_RENDER_MARKERS_SYMBOLIZER_HPP
#define MAPNIK_RENDERER_COMMON_RENDER_MARKERS_SYMBOLIZER_HPP

#include <mapnik/marker.hpp>
#include <mapnik/markers_placement.hpp>
#include <mapnik/renderer_common.hpp>
#include <mapnik/symbolizer_base.hpp>

namespace mapnik {

struct markers_dispatch_params
{
    // placement
    markers_placement_params placement_params;
    marker_placement_enum placement_method;
    value_bool ignore_placement;
    // rendering
    bool snap_to_pixels;
    double scale_factor;
    value_double opacity;

    markers_dispatch_params(box2d<double> const& size,
                            agg::trans_affine const& tr,
                            symbolizer_base const& sym,
                            feature_impl const& feature,
                            attributes const& vars,
                            double scale_factor = 1.0,
                            bool snap_to_pixels = false);
};

struct markers_renderer_context : util::noncopyable
{
    virtual void render_marker(image_rgba8 const& src,
                               markers_dispatch_params const& params,
                               agg::trans_affine const& marker_tr) = 0;

    virtual void render_marker(svg_path_ptr const& src,
                               svg_path_adapter& path,
                               svg::group const& group_attrs,
                               markers_dispatch_params const& params,
                               agg::trans_affine const& marker_tr) = 0;
};

MAPNIK_DECL
void render_markers_symbolizer(markers_symbolizer const& sym,
                               mapnik::feature_impl& feature,
                               proj_transform const& prj_trans,
                               renderer_common const& common,
                               box2d<double> const& clip_box,
                               markers_renderer_context& renderer_context);

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_RENDER_MARKERS_SYMBOLIZER_HPP
