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

#include <mapnik/symbolizer_enumerations.hpp>

namespace mapnik {

// stroke
using line_cap_e_str = detail::EnumStringT<line_cap_enum>;
constexpr detail::EnumMapT<line_cap_enum, 4> line_cap_e_map{{
  line_cap_e_str{line_cap_enum::BUTT_CAP, "butt"},
  line_cap_e_str{line_cap_enum::SQUARE_CAP, "square"},
  line_cap_e_str{line_cap_enum::ROUND_CAP, "round"},
  line_cap_e_str{line_cap_enum::line_cap_enum_MAX, ""},
}};
IMPLEMENT_ENUM(line_cap_e, line_cap_enum)

using line_join_e_str = detail::EnumStringT<line_join_enum>;
constexpr detail::EnumMapT<line_join_enum, 5> line_join_e_map{{
  line_join_e_str{line_join_enum::MITER_JOIN, "miter"},
  line_join_e_str{line_join_enum::MITER_REVERT_JOIN, "miter-revert"},
  line_join_e_str{line_join_enum::ROUND_JOIN, "round"},
  line_join_e_str{line_join_enum::BEVEL_JOIN, "bevel"},
  line_join_e_str{line_join_enum::line_join_enum_MAX, ""},
}};
IMPLEMENT_ENUM(line_join_e, line_join_enum)

// point symbolizer
using point_placement_e_str = detail::EnumStringT<point_placement_enum>;
constexpr detail::EnumMapT<point_placement_enum, 3> point_placement_e_map{{
  point_placement_e_str{point_placement_enum::CENTROID_POINT_PLACEMENT, "centroid"},
  point_placement_e_str{point_placement_enum::INTERIOR_POINT_PLACEMENT, "interior"},
  point_placement_e_str{point_placement_enum::point_placement_enum_MAX, ""},
}};
IMPLEMENT_ENUM(point_placement_e, point_placement_enum)

// line symbolizer
using line_rasterizer_e_str = detail::EnumStringT<line_rasterizer_enum>;
constexpr detail::EnumMapT<line_rasterizer_enum, 3> line_rasterizer_e_map{{
  line_rasterizer_e_str{line_rasterizer_enum::RASTERIZER_FULL, "full"},
  line_rasterizer_e_str{line_rasterizer_enum::RASTERIZER_FAST, "fast"},
  line_rasterizer_e_str{line_rasterizer_enum::line_rasterizer_enum_MAX, ""},
}};
IMPLEMENT_ENUM(line_rasterizer_e, line_rasterizer_enum)

// markers symbolizer
using marker_placement_e_str = detail::EnumStringT<marker_placement_enum>;
constexpr detail::EnumMapT<marker_placement_enum, 8> marker_placement_e_map{{
  marker_placement_e_str{marker_placement_enum::MARKER_POINT_PLACEMENT, "point"},
  marker_placement_e_str{marker_placement_enum::MARKER_INTERIOR_PLACEMENT, "interior"},
  marker_placement_e_str{marker_placement_enum::MARKER_LINE_PLACEMENT, "line"},
  marker_placement_e_str{marker_placement_enum::MARKER_VERTEX_FIRST_PLACEMENT, "vertex-first"},
  marker_placement_e_str{marker_placement_enum::MARKER_VERTEX_LAST_PLACEMENT, "vertex-last"},
  marker_placement_e_str{marker_placement_enum::MARKER_ANGLED_POINT_PLACEMENT, "angled-point"},
  marker_placement_e_str{marker_placement_enum::MARKER_POLYLABEL_PLACEMENT, "polylabel"},
  marker_placement_e_str{marker_placement_enum::marker_placement_enum_MAX, ""},
}};
IMPLEMENT_ENUM(marker_placement_e, marker_placement_enum)

using marker_multi_policy_e_str = detail::EnumStringT<marker_multi_policy_enum>;
constexpr detail::EnumMapT<marker_multi_policy_enum, 4> marker_multi_policy_e_map{{
  marker_multi_policy_e_str{marker_multi_policy_enum::MARKER_EACH_MULTI, "each"},
  marker_multi_policy_e_str{marker_multi_policy_enum::MARKER_WHOLE_MULTI, "whole"},
  marker_multi_policy_e_str{marker_multi_policy_enum::MARKER_LARGEST_MULTI, "largest"},
  marker_multi_policy_e_str{marker_multi_policy_enum::marker_multi_policy_enum_MAX, ""},
}};
IMPLEMENT_ENUM(marker_multi_policy_e, marker_multi_policy_enum)

// debug symbolizer
using debug_symbolizer_mode_e_str = detail::EnumStringT<debug_symbolizer_mode_enum>;
constexpr detail::EnumMapT<debug_symbolizer_mode_enum, 4> debug_symbolizer_mode_e_map{{
  debug_symbolizer_mode_e_str{debug_symbolizer_mode_enum::DEBUG_SYM_MODE_COLLISION, "collision"},
  debug_symbolizer_mode_e_str{debug_symbolizer_mode_enum::DEBUG_SYM_MODE_VERTEX, "vertex"},
  debug_symbolizer_mode_e_str{debug_symbolizer_mode_enum::DEBUG_SYM_MODE_RINGS, "rings"},
  debug_symbolizer_mode_e_str{debug_symbolizer_mode_enum::debug_symbolizer_mode_enum_MAX, ""},
}};
IMPLEMENT_ENUM(debug_symbolizer_mode_e, debug_symbolizer_mode_enum)

// polygon pattern symbolizer
using pattern_alignment_e_str = detail::EnumStringT<pattern_alignment_enum>;
constexpr detail::EnumMapT<pattern_alignment_enum, 3> pattern_alignment_e_map{{
  pattern_alignment_e_str{pattern_alignment_enum::LOCAL_ALIGNMENT, "local"},   // feature
  pattern_alignment_e_str{pattern_alignment_enum::GLOBAL_ALIGNMENT, "global"}, // map
  pattern_alignment_e_str{pattern_alignment_enum::pattern_alignment_enum_MAX, ""},
}};
IMPLEMENT_ENUM(pattern_alignment_e, pattern_alignment_enum)

// text
using halo_rasterizer_e_str = detail::EnumStringT<halo_rasterizer_enum>;
constexpr detail::EnumMapT<halo_rasterizer_enum, 3> halo_rasterizer_e_map{{
  halo_rasterizer_e_str{halo_rasterizer_enum::HALO_RASTERIZER_FULL, "full"},
  halo_rasterizer_e_str{halo_rasterizer_enum::HALO_RASTERIZER_FAST, "fast"},
  halo_rasterizer_e_str{halo_rasterizer_enum::halo_rasterizer_enum_MAX, ""},
}};
IMPLEMENT_ENUM(halo_rasterizer_e, halo_rasterizer_enum)

using label_placement_e_str = detail::EnumStringT<label_placement_enum>;
constexpr detail::EnumMapT<label_placement_enum, 8> label_placement_e_map{{
  label_placement_e_str{label_placement_enum::POINT_PLACEMENT, "point"},
  label_placement_e_str{label_placement_enum::LINE_PLACEMENT, "line"},
  label_placement_e_str{label_placement_enum::VERTEX_PLACEMENT, "vertex"},
  label_placement_e_str{label_placement_enum::INTERIOR_PLACEMENT, "interior"},
  label_placement_e_str{label_placement_enum::POLYLABEL_PLACEMENT, "polylabel"},
  label_placement_e_str{label_placement_enum::GRID_PLACEMENT, "grid"},
  label_placement_e_str{label_placement_enum::ALTERNATING_GRID_PLACEMENT, "alternating-grid"},
  label_placement_e_str{label_placement_enum::label_placement_enum_MAX, ""},
}};
IMPLEMENT_ENUM(label_placement_e, label_placement_enum)

using vertical_alignment_e_str = detail::EnumStringT<vertical_alignment_enum>;
constexpr detail::EnumMapT<vertical_alignment_enum, 5> vertical_alignment_e_map{{
  vertical_alignment_e_str{vertical_alignment_enum::V_TOP, "top"},
  vertical_alignment_e_str{vertical_alignment_enum::V_MIDDLE, "middle"},
  vertical_alignment_e_str{vertical_alignment_enum::V_BOTTOM, "bottom"},
  vertical_alignment_e_str{vertical_alignment_enum::V_AUTO, "auto"},
  vertical_alignment_e_str{vertical_alignment_enum::vertical_alignment_enum_MAX, ""},
}};
IMPLEMENT_ENUM(vertical_alignment_e, vertical_alignment_enum)

using horizontal_alignment_e_str = detail::EnumStringT<horizontal_alignment_enum>;
constexpr detail::EnumMapT<horizontal_alignment_enum, 6> horizontal_alignment_e_map{{
  horizontal_alignment_e_str{horizontal_alignment_enum::H_LEFT, "left"},
  horizontal_alignment_e_str{horizontal_alignment_enum::H_MIDDLE, "middle"},
  horizontal_alignment_e_str{horizontal_alignment_enum::H_RIGHT, "right"},
  horizontal_alignment_e_str{horizontal_alignment_enum::H_AUTO, "auto"},
  horizontal_alignment_e_str{horizontal_alignment_enum::H_ADJUST, "adjust"},
  horizontal_alignment_e_str{horizontal_alignment_enum::horizontal_alignment_enum_MAX, ""},
}};
IMPLEMENT_ENUM(horizontal_alignment_e, horizontal_alignment_enum)

using justify_alignment_e_str = detail::EnumStringT<justify_alignment_enum>;
constexpr detail::EnumMapT<justify_alignment_enum, 5> justify_alignment_e_map{{
  justify_alignment_e_str{justify_alignment_enum::J_LEFT, "left"},
  justify_alignment_e_str{justify_alignment_enum::J_MIDDLE, "center"}, // not 'middle' in order to match CSS
  justify_alignment_e_str{justify_alignment_enum::J_RIGHT, "right"},
  justify_alignment_e_str{justify_alignment_enum::J_AUTO, "auto"},
  justify_alignment_e_str{justify_alignment_enum::justify_alignment_enum_MAX, ""},
}};
IMPLEMENT_ENUM(justify_alignment_e, justify_alignment_enum)

using text_transform_e_str = detail::EnumStringT<text_transform_enum>;
constexpr detail::EnumMapT<text_transform_enum, 6> text_transform_e_map{{
  text_transform_e_str{text_transform_enum::NONE, "none"},
  text_transform_e_str{text_transform_enum::UPPERCASE, "uppercase"},
  text_transform_e_str{text_transform_enum::LOWERCASE, "lowercase"},
  text_transform_e_str{text_transform_enum::CAPITALIZE, "capitalize"},
  text_transform_e_str{text_transform_enum::REVERSE, "reverse"},
  text_transform_e_str{text_transform_enum::text_transform_enum_MAX, ""},
}};
IMPLEMENT_ENUM(text_transform_e, text_transform_enum)

using text_upright_e_str = detail::EnumStringT<text_upright_enum>;
constexpr detail::EnumMapT<text_upright_enum, 7> text_upright_e_map{{
  text_upright_e_str{text_upright_enum::UPRIGHT_AUTO, "auto"},
  text_upright_e_str{text_upright_enum::UPRIGHT_AUTO_DOWN, "auto-down"},
  text_upright_e_str{text_upright_enum::UPRIGHT_LEFT, "left"},
  text_upright_e_str{text_upright_enum::UPRIGHT_RIGHT, "right"},
  text_upright_e_str{text_upright_enum::UPRIGHT_LEFT_ONLY, "left-only"},
  text_upright_e_str{text_upright_enum::UPRIGHT_RIGHT_ONLY, "right-only"},
  text_upright_e_str{text_upright_enum::text_upright_enum_MAX, ""},
}};
IMPLEMENT_ENUM(text_upright_e, text_upright_enum)

using direction_e_str = detail::EnumStringT<direction_enum>;
constexpr detail::EnumMapT<direction_enum, 9> direction_e_map{{
  direction_e_str{direction_enum::DIRECTION_LEFT, "left"},
  direction_e_str{direction_enum::DIRECTION_RIGHT, "right"},
  direction_e_str{direction_enum::DIRECTION_LEFT_ONLY, "left-only"},
  direction_e_str{direction_enum::DIRECTION_RIGHT_ONLY, "right-only"},
  direction_e_str{direction_enum::DIRECTION_AUTO, "auto"},
  direction_e_str{direction_enum::DIRECTION_AUTO_DOWN, "auto-down"},
  direction_e_str{direction_enum::DIRECTION_UP, "up"},
  direction_e_str{direction_enum::DIRECTION_DOWN, "down"},
  direction_e_str{direction_enum::direction_enum_MAX, ""},
}};
IMPLEMENT_ENUM(direction_e, direction_enum)

using gamma_method_e_str = detail::EnumStringT<gamma_method_enum>;
constexpr detail::EnumMapT<gamma_method_enum, 6> gamma_method_e_map{{
  gamma_method_e_str{gamma_method_enum::GAMMA_POWER, "power"},         // agg::gamma_power
  gamma_method_e_str{gamma_method_enum::GAMMA_LINEAR, "linear"},       // agg::gamma_linear
  gamma_method_e_str{gamma_method_enum::GAMMA_NONE, "none"},           // agg::gamma_none
  gamma_method_e_str{gamma_method_enum::GAMMA_THRESHOLD, "threshold"}, // agg::gamma_threshold
  gamma_method_e_str{gamma_method_enum::GAMMA_MULTIPLY, "multiply"},   // agg::gamma_multiply"
  gamma_method_e_str{gamma_method_enum::gamma_method_enum_MAX, ""},
}};
IMPLEMENT_ENUM(gamma_method_e, gamma_method_enum)

using line_pattern_e_str = detail::EnumStringT<line_pattern_enum>;
constexpr detail::EnumMapT<line_pattern_enum, 3> line_pattern_e_map{{
  line_pattern_e_str{line_pattern_enum::LINE_PATTERN_WARP, "warp"},
  line_pattern_e_str{line_pattern_enum::LINE_PATTERN_REPEAT, "repeat"},
  line_pattern_e_str{line_pattern_enum::line_pattern_enum_MAX, ""},
}};
IMPLEMENT_ENUM(line_pattern_e, line_pattern_enum)

using smooth_algorithm_e_str = detail::EnumStringT<smooth_algorithm_enum>;
constexpr detail::EnumMapT<smooth_algorithm_enum, 3> smooth_algorithm_e_map{{
  smooth_algorithm_e_str{smooth_algorithm_enum::SMOOTH_ALGORITHM_BASIC, "basic"},
  smooth_algorithm_e_str{smooth_algorithm_enum::SMOOTH_ALGORITHM_ADAPTIVE, "adaptive"},
  smooth_algorithm_e_str{smooth_algorithm_enum::smooth_algorithm_enum_MAX, ""},
}};
IMPLEMENT_ENUM(smooth_algorithm_e, smooth_algorithm_enum)

} // namespace mapnik
