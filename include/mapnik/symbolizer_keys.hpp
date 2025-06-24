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

#ifndef MAPNIK_SYMBOLIZER_KEYS_HPP
#define MAPNIK_SYMBOLIZER_KEYS_HPP

#include <cstdint>

namespace mapnik {

enum class keys : std::uint8_t {
    gamma = 0,
    gamma_method,
    opacity,
    alignment,
    offset,
    comp_op,
    clip,
    fill,
    fill_opacity,
    stroke,
    stroke_width,
    stroke_opacity,
    stroke_linejoin,
    stroke_linecap,
    stroke_gamma,
    stroke_gamma_method,
    stroke_dashoffset,
    stroke_dasharray,
    stroke_miterlimit,
    geometry_transform,
    line_rasterizer,
    image_transform,
    spacing,
    spacing_offset,
    max_error,
    allow_overlap,
    ignore_placement,
    width,
    height,
    file,
    shield_dx,
    shield_dy,
    unlock_image,
    mode,
    scaling,
    filter_factor,
    mesh_size,
    premultiplied,
    smooth,
    smooth_algorithm,
    simplify_algorithm,
    simplify_tolerance,
    halo_rasterizer,
    text_placements_,
    label_placement,
    markers_placement_type,
    markers_multipolicy,
    point_placement_type,
    colorizer,
    halo_transform,
    num_columns,
    start_column,
    repeat_key,
    group_properties,
    largest_box_only,
    minimum_path_length,
    halo_comp_op,
    text_transform,
    horizontal_alignment,
    justify_alignment,
    vertical_alignment,
    upright,
    direction,
    avoid_edges,
    ff_settings,
    extend,
    line_pattern,
    MAX_SYMBOLIZER_KEY
};

}

#endif // MAPNIK_SYMBOLIZER_KEYS_HPP
