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

#ifndef MAPNIK_SYMBOLIZER_ENUMERATIONS_HPP
#define MAPNIK_SYMBOLIZER_ENUMERATIONS_HPP

#include <mapnik/enumeration.hpp>

namespace mapnik {

enum line_cap_enum : std::uint8_t
{
    BUTT_CAP,
    SQUARE_CAP,
    ROUND_CAP,
    line_cap_enum_MAX
};

DEFINE_ENUM( line_cap_e, line_cap_enum );

enum line_join_enum : std::uint8_t
{
    MITER_JOIN,
    MITER_REVERT_JOIN,
    ROUND_JOIN,
    BEVEL_JOIN,
    line_join_enum_MAX
};

DEFINE_ENUM( line_join_e, line_join_enum );

enum line_rasterizer_enum : std::uint8_t
{
    RASTERIZER_FULL,           // agg::renderer_scanline_aa_solid
    RASTERIZER_FAST,           // agg::rasterizer_outline_aa, twice as fast but only good for thin lines
    line_rasterizer_enum_MAX
};

DEFINE_ENUM( line_rasterizer_e, line_rasterizer_enum );


enum halo_rasterizer_enum : std::uint8_t
{
    HALO_RASTERIZER_FULL,
    HALO_RASTERIZER_FAST,
    halo_rasterizer_enum_MAX
};

DEFINE_ENUM(halo_rasterizer_e, halo_rasterizer_enum);

enum point_placement_enum : std::uint8_t
{
    CENTROID_POINT_PLACEMENT,
    INTERIOR_POINT_PLACEMENT,
    point_placement_enum_MAX
};

DEFINE_ENUM( point_placement_e, point_placement_enum );

enum pattern_alignment_enum : std::uint8_t
{
    LOCAL_ALIGNMENT,
    GLOBAL_ALIGNMENT,
    pattern_alignment_enum_MAX
};

DEFINE_ENUM( pattern_alignment_e, pattern_alignment_enum );

enum debug_symbolizer_mode_enum : std::uint8_t
{
    DEBUG_SYM_MODE_COLLISION,
    DEBUG_SYM_MODE_VERTEX,
    DEBUG_SYM_MODE_RINGS,
    debug_symbolizer_mode_enum_MAX
};

DEFINE_ENUM( debug_symbolizer_mode_e, debug_symbolizer_mode_enum );


// markers
// TODO - consider merging with text_symbolizer label_placement_e
enum marker_placement_enum : std::uint8_t
{
    MARKER_POINT_PLACEMENT,
    MARKER_INTERIOR_PLACEMENT,
    MARKER_LINE_PLACEMENT,
    MARKER_VERTEX_FIRST_PLACEMENT,
    MARKER_VERTEX_LAST_PLACEMENT,
    marker_placement_enum_MAX
};

DEFINE_ENUM( marker_placement_e, marker_placement_enum );

enum marker_multi_policy_enum : std::uint8_t
{
    MARKER_EACH_MULTI, // each component in a multi gets its marker
    MARKER_WHOLE_MULTI, // consider all components of a multi as a whole
    MARKER_LARGEST_MULTI, // only the largest component of a multi gets a marker
    marker_multi_policy_enum_MAX
};

DEFINE_ENUM( marker_multi_policy_e, marker_multi_policy_enum );

enum text_transform_enum : std::uint8_t
{
    NONE = 0,
    UPPERCASE,
    LOWERCASE,
    CAPITALIZE,
    REVERSE,
    text_transform_enum_MAX
};

DEFINE_ENUM(text_transform_e, text_transform_enum);

enum label_placement_enum : std::uint8_t
{
    POINT_PLACEMENT,
    LINE_PLACEMENT,
    VERTEX_PLACEMENT,
    INTERIOR_PLACEMENT,
    GRID_PLACEMENT,
    ALTERNATING_GRID_PLACEMENT,
    label_placement_enum_MAX
};

DEFINE_ENUM(label_placement_e, label_placement_enum);

enum vertical_alignment_enum : std::uint8_t
{
    V_TOP = 0,
    V_MIDDLE,
    V_BOTTOM,
    V_AUTO,
    vertical_alignment_enum_MAX
};

DEFINE_ENUM(vertical_alignment_e, vertical_alignment_enum);

enum horizontal_alignment_enum : std::uint8_t
{
    H_LEFT = 0,
    H_MIDDLE,
    H_RIGHT,
    H_AUTO,
    H_ADJUST,
    horizontal_alignment_enum_MAX
};

DEFINE_ENUM(horizontal_alignment_e, horizontal_alignment_enum);

enum justify_alignment_enum : std::uint8_t
{
    J_LEFT = 0,
    J_MIDDLE,
    J_RIGHT,
    J_AUTO,
    justify_alignment_enum_MAX
};

DEFINE_ENUM(justify_alignment_e, justify_alignment_enum);

enum text_upright_enum : std::uint8_t
{
    UPRIGHT_AUTO,
    UPRIGHT_AUTO_DOWN,
    UPRIGHT_LEFT,
    UPRIGHT_RIGHT,
    UPRIGHT_LEFT_ONLY,
    UPRIGHT_RIGHT_ONLY,
    text_upright_enum_MAX
};

DEFINE_ENUM(text_upright_e, text_upright_enum);

enum direction_enum : std::uint8_t
{
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_LEFT_ONLY,
    DIRECTION_RIGHT_ONLY,
    DIRECTION_AUTO,
    DIRECTION_AUTO_DOWN,
    DIRECTION_UP,
    DIRECTION_DOWN,
    direction_enum_MAX
};

DEFINE_ENUM(direction_e, direction_enum);

enum gamma_method_enum : std::uint8_t
{
    GAMMA_POWER, //agg::gamma_power
    GAMMA_LINEAR, //agg::gamma_linear
    GAMMA_NONE, //agg::gamma_none
    GAMMA_THRESHOLD, //agg::gamma_threshold
    GAMMA_MULTIPLY, //agg::gamma_multiply
    gamma_method_enum_MAX
};

DEFINE_ENUM (gamma_method_e, gamma_method_enum );

enum line_pattern_enum : std::uint8_t
{
    LINE_PATTERN_WARP,
    LINE_PATTERN_REPEAT,
    line_pattern_enum_MAX
};

DEFINE_ENUM (line_pattern_e, line_pattern_enum );

}

#endif //MAPNIK_SYMBOLIZER_ENUMERATIONS_HPP
