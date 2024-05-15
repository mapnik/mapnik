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

#include <mapnik/symbolizer.hpp>
#include <mapnik/color.hpp>
#include <mapnik/simplify.hpp>

// stl
#include <algorithm>

namespace mapnik {

static constexpr std::uint8_t const_max_key = static_cast<std::uint8_t>(keys::MAX_SYMBOLIZER_KEY);

// tuple -> name, default value, enumeration to string converter lambda, target property type
static const property_meta_type key_meta[const_max_key] = {
  property_meta_type{"gamma", nullptr, property_types::target_double},
  property_meta_type{"gamma-method", nullptr, property_types::target_gamma_method},
  property_meta_type{"opacity", nullptr, property_types::target_double},
  property_meta_type{
    "alignment",
    [](enumeration_wrapper e) { return pattern_alignment_e(pattern_alignment_enum(e.value)).as_string(); },
    property_types::target_pattern_alignment},
  property_meta_type{"offset", nullptr, property_types::target_double},
  property_meta_type{"comp-op",
                     [](enumeration_wrapper e) { return *comp_op_to_string(composite_mode_e(e.value)); },
                     property_types::target_comp_op},
  property_meta_type{"clip", nullptr, property_types::target_bool},
  property_meta_type{"fill", nullptr, property_types::target_color},
  property_meta_type{"fill-opacity", nullptr, property_types::target_double},
  property_meta_type{"stroke", nullptr, property_types::target_color},
  property_meta_type{"stroke-width", nullptr, property_types::target_double},
  property_meta_type{"stroke-opacity", nullptr, property_types::target_double},
  property_meta_type{"stroke-linejoin",
                     [](enumeration_wrapper e) { return line_join_e(line_join_enum(e.value)).as_string(); },
                     property_types::target_line_join},
  property_meta_type{"stroke-linecap",
                     [](enumeration_wrapper e) { return line_cap_e(line_cap_enum(e.value)).as_string(); },
                     property_types::target_line_cap},
  property_meta_type{"stroke-gamma", nullptr, property_types::target_double},
  property_meta_type{"stroke-gamma-method", nullptr, property_types::target_gamma_method},
  property_meta_type{"stroke-dashoffset", nullptr, property_types::target_double},
  property_meta_type{"stroke-dasharray", nullptr, property_types::target_dash_array},
  property_meta_type{"stroke-miterlimit", nullptr, property_types::target_double},
  property_meta_type{"geometry-transform", nullptr, property_types::target_transform},
  // TODO - should be called 'line-rasterizer' but for back compat with 2.3.x we keep as 'rasterizer'
  // https://github.com/mapnik/mapnik/issues/2503
  property_meta_type{"rasterizer",
                     [](enumeration_wrapper e) { return line_rasterizer_e(line_rasterizer_enum(e.value)).as_string(); },
                     property_types::target_double},
  property_meta_type{"transform", nullptr, property_types::target_transform},
  property_meta_type{"spacing", nullptr, property_types::target_double},
  property_meta_type{"spacing-offset", nullptr, property_types::target_double},
  property_meta_type{"max-error", nullptr, property_types::target_double},
  property_meta_type{"allow-overlap", nullptr, property_types::target_bool},
  property_meta_type{"ignore-placement", nullptr, property_types::target_bool},
  property_meta_type{"width", nullptr, property_types::target_double},
  property_meta_type{"height", nullptr, property_types::target_double},
  property_meta_type{"file", nullptr, property_types::target_string},
  property_meta_type{"shield-dx", nullptr, property_types::target_double},
  property_meta_type{"shield-dy", nullptr, property_types::target_double},
  property_meta_type{"unlock-image", nullptr, property_types::target_bool},
  property_meta_type{"mode", nullptr, property_types::target_double},
  property_meta_type{"scaling",
                     [](enumeration_wrapper e) { return *scaling_method_to_string(scaling_method_e(e.value)); },
                     property_types::target_scaling_method},
  property_meta_type{"filter-factor", nullptr, property_types::target_double},
  property_meta_type{"mesh-size", nullptr, property_types::target_double},
  property_meta_type{"premultiplied", nullptr, property_types::target_bool},
  property_meta_type{"smooth", nullptr, property_types::target_double},
  property_meta_type{
    "smooth-algorithm",
    [](enumeration_wrapper e) { return smooth_algorithm_e(smooth_algorithm_enum(e.value)).as_string(); },
    property_types::target_smooth_algorithm},
  property_meta_type{"simplify-algorithm",
                     [](enumeration_wrapper e) { return *simplify_algorithm_to_string(simplify_algorithm_e(e.value)); },
                     property_types::target_simplify_algorithm},
  property_meta_type{"simplify", nullptr, property_types::target_double},
  property_meta_type{"halo-rasterizer",
                     [](enumeration_wrapper e) { return halo_rasterizer_e(halo_rasterizer_enum(e.value)).as_string(); },
                     property_types::target_halo_rasterizer},
  property_meta_type{"text-placements", nullptr, property_types::target_double},
  property_meta_type{"placement",
                     [](enumeration_wrapper e) { return label_placement_e(label_placement_enum(e.value)).as_string(); },
                     property_types::target_placement},
  property_meta_type{
      "placement",
      [](enumeration_wrapper e) { return marker_placement_e(marker_placement_enum(e.value)).as_string(); },
      property_types::target_markers_placement},
  property_meta_type{
      "multi-policy",
      [](enumeration_wrapper e) { return marker_multi_policy_e(marker_multi_policy_enum(e.value)).as_string(); },
      property_types::target_markers_multipolicy},
  property_meta_type{"placement",
      [](enumeration_wrapper e) { return point_placement_e(point_placement_enum(e.value)).as_string(); },
      property_types::target_placement},
  property_meta_type{"colorizer", nullptr, property_types::target_colorizer},
  property_meta_type{"halo-transform", nullptr, property_types::target_transform},
  property_meta_type{"num-columns", nullptr, property_types::target_integer},
  property_meta_type{"start-column", nullptr, property_types::target_integer},
  property_meta_type{"repeat-key", nullptr, property_types::target_repeat_key},
  property_meta_type{"symbolizer-properties", nullptr, property_types::target_group_symbolizer_properties},
  property_meta_type{"largest-box-only", nullptr, property_types::target_bool},
  property_meta_type{"minimum-path-length", nullptr, property_types::target_double},
  property_meta_type{"halo-comp-op",
                     [](enumeration_wrapper e) { return *comp_op_to_string(composite_mode_e(e.value)); },
                     property_types::target_halo_comp_op},
  property_meta_type{"text-transform",
                     [](enumeration_wrapper e) { return text_transform_e(text_transform_enum(e.value)).as_string(); },
                     property_types::target_text_transform},
  property_meta_type{
    "horizontal-alignment",
    [](enumeration_wrapper e) { return horizontal_alignment_e(horizontal_alignment_enum(e.value)).as_string(); },
    property_types::target_horizontal_alignment},
  property_meta_type{
    "justify-alignment",
    [](enumeration_wrapper e) { return justify_alignment_e(justify_alignment_enum(e.value)).as_string(); },
    property_types::target_justify_alignment},
  property_meta_type{
    "vertical-alignment",
    [](enumeration_wrapper e) { return vertical_alignment_e(vertical_alignment_enum(e.value)).as_string(); },
    property_types::target_vertical_alignment},
  property_meta_type{"upright",
                     [](enumeration_wrapper e) { return text_upright_e(text_upright_enum(e.value)).as_string(); },
                     property_types::target_upright},
  property_meta_type{"direction",
                     [](enumeration_wrapper e) { return direction_e(direction_enum(e.value)).as_string(); },
                     property_types::target_direction},
  property_meta_type{"avoid-edges", nullptr, property_types::target_bool},
  property_meta_type{"font-feature-settings", nullptr, property_types::target_font_feature_settings},
  property_meta_type{"extend", nullptr, property_types::target_double},
  property_meta_type{"line-pattern",
                     [](enumeration_wrapper e) { return line_pattern_e(line_pattern_enum(e.value)).as_string(); },
                     property_types::target_line_pattern},

};

property_meta_type const& get_meta(mapnik::keys key)
{
    return key_meta[static_cast<int>(key)];
}

mapnik::keys get_key(std::string const& name)
{
    std::string name_copy(name);
    std::replace(name_copy.begin(), name_copy.end(), '_', '-');
    for (unsigned i = 0; i < const_max_key; ++i)
    {
        property_meta_type const& item = key_meta[i];
        if (name_copy == std::get<0>(item))
        {
            return static_cast<mapnik::keys>(i);
        }
    }
    throw std::runtime_error("no key found for '" + name + "'");
    return static_cast<mapnik::keys>(0);
}

} // namespace mapnik
