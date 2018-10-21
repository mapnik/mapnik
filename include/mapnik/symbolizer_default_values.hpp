/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_SYMBOLIZER_DEFAULT_VALUES_HPP
#define MAPNIK_SYMBOLIZER_DEFAULT_VALUES_HPP

#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/simplify.hpp>
#include <mapnik/color.hpp>
#include <mapnik/value_types.hpp>
#include <type_traits>


namespace mapnik {

template <typename T, keys key>
struct symbolizer_default {};

// specializations for properties
// gamma
template <>
struct symbolizer_default<value_double, keys::gamma>
{
    static value_double value() { return 1.0; }
};

// gamma-method
template <>
struct symbolizer_default<gamma_method_enum, keys::gamma_method>
{
    static gamma_method_enum value() { return GAMMA_POWER; }
};

// opacity
template <>
struct symbolizer_default<value_double, keys::opacity>
{
    static value_double value() { return 1.0; }
};

// alignment
template <>
struct symbolizer_default<pattern_alignment_enum, keys::alignment>
{
    static pattern_alignment_enum value() { return GLOBAL_ALIGNMENT; }
};

// offset
template <>
struct symbolizer_default<value_double, keys::offset>
{
    static value_double value() { return 0.0; }
};

// comp-op
template <>
struct symbolizer_default<composite_mode_e, keys::comp_op>
{
    static composite_mode_e value() { return src_over; }
};

// clip
template <>
struct symbolizer_default<value_bool, keys::clip>
{
    static value_bool value() { return false; }
};

// fill
template <>
struct symbolizer_default<color, keys::fill>
{
    static color value() { return color(128,128,128); }
};

// fill-opacity
template <>
struct symbolizer_default<value_double, keys::fill_opacity>
{
    static value_double value() { return 1.0; }
};

// stroke
template <>
struct symbolizer_default<color, keys::stroke>
{
    static color value() { return color(0,0,0); }
};

// stroke-width
template <>
struct symbolizer_default<value_double, keys::stroke_width>
{
    static value_double value() { return 1.0; }
};

// stroke-opacity
template <>
struct symbolizer_default<value_double, keys::stroke_opacity>
{
    static value_double value() { return 1.0; }
};

// stroke-linejoin
template <>
struct symbolizer_default<line_join_enum, keys::stroke_linejoin>
{
    static line_join_enum value() { return MITER_JOIN; }
};

// stroke-linecap
template <>
struct symbolizer_default<line_cap_enum, keys::stroke_linecap>
{
    static line_cap_enum value() { return BUTT_CAP; }
};

// stroke-gamma
template <>
struct symbolizer_default<value_double, keys::stroke_gamma>
{
    static value_double value() { return 1.0; }
};

// stroke-gamma-method
template <>
struct symbolizer_default<gamma_method_enum, keys::stroke_gamma_method>
{
    static gamma_method_enum value() { return GAMMA_POWER; }
};

// stroke-dashoffset
template <>
struct symbolizer_default<value_integer, keys::stroke_dashoffset>
{
    static value_integer value() { return 0; }
};

// stroke-dasharray

// stroke-miterlimit
template <>
struct symbolizer_default<value_double, keys::stroke_miterlimit>
{
    static value_double value() { return 4.0; }
};

// geometry-transform

// rasterizer
template <>
struct symbolizer_default<line_rasterizer_enum, keys::line_rasterizer>
{
    static line_rasterizer_enum value() { return RASTERIZER_FULL; }
};

// transform

// spacing
template <>
struct symbolizer_default<value_double, keys::spacing>
{
    static value_double value() { return 100.0; }
};

// max-error
template <>
struct symbolizer_default<value_double, keys::max_error>
{
    static value_double value() { return 0.2; }
};

// allow-overlap
template <>
struct symbolizer_default<value_bool, keys::allow_overlap>
{
    static value_bool value() { return false; }
};

// ignore-placement
template <>
struct symbolizer_default<value_bool, keys::ignore_placement>
{
    static value_bool value() { return false; }
};

// width
template <>
struct symbolizer_default<value_double, keys::width>
{
    static value_double value() { return 0.0; }
};

// height
template <>
struct symbolizer_default<value_double, keys::height>
{
    static value_double value() { return 0.0; }
};

// file ??
template <>
struct symbolizer_default<std::string, keys::file>
{
    static std::string value() { return ""; }
};

// shield-dx
template <>
struct symbolizer_default<value_double, keys::shield_dx>
{
    static value_double value() { return 0.0; }
};

// shield-dy
template <>
struct symbolizer_default<value_double, keys::shield_dy>
{
    static value_double value() { return 0.0; }
};

// unlock-image
template <>
struct symbolizer_default<value_bool, keys::unlock_image>
{
    static value_bool value() { return false; }
};

// mode
template <>
struct symbolizer_default<value_bool, keys::mode>
{
    static value_bool value() { return false; }
};

// scaling
template <>
struct symbolizer_default<value_double, keys::scaling>
{
    static value_double value() { return 1.0; }
};

// filter-factor
template <>
struct symbolizer_default<value_double, keys::filter_factor>
{
    static value_double value() { return 1.0; }
};

// mesh-size
template <>
struct symbolizer_default<value_integer, keys::mesh_size>
{
    static value_integer value() { return 0; }
};

// premultiplied
template <>
struct symbolizer_default<value_bool, keys::premultiplied>
{
    static value_bool value() { return false; }
};

// smooth
template <>
struct symbolizer_default<value_double, keys::smooth>
{
    static value_double value() { return 0.0; }
};

// simplify-algorithm
template <>
struct symbolizer_default<simplify_algorithm_e, keys::simplify_algorithm>
{
    static simplify_algorithm_e value() { return radial_distance; }
};

// simplify
template <>
struct symbolizer_default<value_double, keys::simplify_tolerance>
{
    static value_double value() { return 0.0; }
};

// halo-rasterizer
template <>
struct symbolizer_default<halo_rasterizer_enum, keys::halo_rasterizer>
{
    static halo_rasterizer_enum value() { return HALO_RASTERIZER_FULL; }
};

// text-placements

// placement (point-placement-type FIXME)
template <>
struct symbolizer_default<point_placement_enum, keys::point_placement_type>
{
    static point_placement_enum value() { return CENTROID_POINT_PLACEMENT; }
};

// marker placement
template <>
struct symbolizer_default<marker_placement_enum, keys::markers_placement_type>
{
    static marker_placement_enum value() { return MARKER_POINT_PLACEMENT; }
};

// multi-policy
template <>
struct symbolizer_default<marker_multi_policy_enum, keys::markers_multipolicy>
{
    static marker_multi_policy_enum value() { return  MARKER_EACH_MULTI; }
};

// direction
template <>
struct symbolizer_default<direction_enum, keys::direction>
{
    static direction_enum value() { return DIRECTION_RIGHT; }
};

// placement

// colorizer

// halo-transform

// num-columns
// start-column
// repeat-key
// symbolizer-properties ??
// largest-box-only
// minimum-path-length
// halo-comp-op
// text-transform
// horizontal-alignment
// justify-alignment
// vertical-alignment
// upright

// avoid-edges
template <>
struct symbolizer_default<value_bool, keys::avoid_edges>
{
    static value_bool value() { return false; }
};

// font-feature-settings

// line-pattern
template <>
struct symbolizer_default<line_pattern_enum, keys::line_pattern>
{
    static line_pattern_enum value() { return LINE_PATTERN_WARP; }
};

} // namespace mapnik

#endif // MAPNIK_SYMBOLIZER_DEFAULT_VALUES_HPP
