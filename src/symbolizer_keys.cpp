/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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

// boost
#include <boost/algorithm/string/replace.hpp>  // for replace

namespace mapnik {

static constexpr std::uint8_t const_max_key = static_cast<std::uint8_t>(keys::MAX_SYMBOLIZER_KEY);

// tuple -> name, default value, enumeration to string converter lambda, target property type
static const property_meta_type key_meta[const_max_key] =
{
    property_meta_type{ "gamma", 1.0, nullptr, property_types::target_double},
    property_meta_type{ "gamma-method", static_cast<value_integer>(GAMMA_POWER), nullptr, property_types::target_gamma_method},
    property_meta_type{ "opacity", 1.0, nullptr, property_types::target_double},
    property_meta_type{ "alignment", enumeration_wrapper(LOCAL_ALIGNMENT),
                        [](enumeration_wrapper e) { return enumeration<pattern_alignment_enum,pattern_alignment_enum_MAX>(pattern_alignment_enum(e.value)).as_string();}, property_types::target_pattern_alignment},
    property_meta_type{ "offset", 0.0, nullptr, property_types::target_double},
    property_meta_type{ "comp-op", enumeration_wrapper(src_over),
                        [](enumeration_wrapper e) { return *comp_op_to_string(composite_mode_e(e.value)); }, property_types::target_comp_op},
    property_meta_type{ "clip", false, nullptr, property_types::target_bool},
    property_meta_type{ "fill", mapnik::color(128,128,128), nullptr, property_types::target_color},
    property_meta_type{ "fill-opacity", 1.0 , nullptr, property_types::target_double},
    property_meta_type{ "stroke", mapnik::color(0,0,0), nullptr, property_types::target_color},
    property_meta_type{ "stroke-width", 1.0 , nullptr, property_types::target_double},
    property_meta_type{ "stroke-opacity", 1.0, nullptr, property_types::target_double},
    property_meta_type{ "stroke-linejoin", enumeration_wrapper(MITER_JOIN),
                        [](enumeration_wrapper e) { return enumeration<line_join_enum,line_join_enum_MAX>(line_join_enum(e.value)).as_string();}, property_types::target_line_join },
    property_meta_type{ "stroke-linecap", enumeration_wrapper(BUTT_CAP),
                        [](enumeration_wrapper e) { return enumeration<line_cap_enum,line_cap_enum_MAX>(line_cap_enum(e.value)).as_string();}, property_types::target_line_cap },
    property_meta_type{ "stroke-gamma", 1.0, nullptr, property_types::target_double },
    property_meta_type{ "stroke-gamma-method",static_cast<value_integer>(GAMMA_POWER), nullptr, property_types::target_gamma_method },
    property_meta_type{ "stroke-dashoffset", static_cast<value_integer>(0), nullptr, property_types::target_double },
    property_meta_type{ "stroke-dasharray", false, nullptr, property_types::target_dash_array },
    property_meta_type{ "stroke-miterlimit", 4.0, nullptr, property_types::target_double },
    property_meta_type{ "geometry-transform", false, nullptr, property_types::target_transform },
    property_meta_type{ "line-rasterizer", enumeration_wrapper(RASTERIZER_FULL),
                        [](enumeration_wrapper e) { return enumeration<line_rasterizer_enum,line_rasterizer_enum_MAX>(line_rasterizer_enum(e.value)).as_string();}, property_types::target_double },
    property_meta_type{ "transform", false, nullptr, property_types::target_transform },
    property_meta_type{ "spacing", 0.0, nullptr, property_types::target_double },
    property_meta_type{ "max-error", 0.0, nullptr, property_types::target_double },
    property_meta_type{ "allow-overlap",false, nullptr, property_types::target_bool },
    property_meta_type{ "ignore-placement", false, nullptr, property_types::target_bool },
    property_meta_type{ "width",0.0, nullptr, property_types::target_double },
    property_meta_type{ "height",0.0, nullptr, property_types::target_double },
    property_meta_type{ "file", "", nullptr, property_types::target_string },
    property_meta_type{ "shield-dx", 0.0, nullptr, property_types::target_double },
    property_meta_type{ "shield-dy", 0.0, nullptr, property_types::target_double },
    property_meta_type{ "unlock-image",false, nullptr, property_types::target_bool },
    property_meta_type{ "text-opacity", 1.0, nullptr, property_types::target_double },
    property_meta_type{ "mode",false, nullptr, property_types::target_double },
    property_meta_type{ "scaling", 1.0, nullptr, property_types::target_double },
    property_meta_type{ "filter-factor", 1.0, nullptr, property_types::target_double },
    property_meta_type{ "mesh-size", static_cast<value_integer>(0), nullptr, property_types::target_double },
    property_meta_type{ "premultiplied", false, nullptr, property_types::target_bool },
    property_meta_type{ "smooth", false, nullptr, property_types::target_double },
    property_meta_type{ "simplify-algorithm", enumeration_wrapper(radial_distance),
                        [](enumeration_wrapper e) { return *simplify_algorithm_to_string(simplify_algorithm_e(e.value));}, property_types::target_simplify_algorithm },
    property_meta_type{ "simplify", 0.0, nullptr, property_types::target_double },
    property_meta_type{ "halo-rasterizer", enumeration_wrapper(HALO_RASTERIZER_FULL),
                        [](enumeration_wrapper e) { return enumeration<halo_rasterizer_enum,halo_rasterizer_enum_MAX>(halo_rasterizer_enum(e.value)).as_string();}, property_types::target_halo_rasterizer },
    property_meta_type{ "text-placements", false, nullptr, property_types::target_double },
    property_meta_type{ "placement", enumeration_wrapper(POINT_PLACEMENT),
                        [](enumeration_wrapper e)
                        { return enumeration<label_placement_enum,label_placement_enum_MAX>(label_placement_enum(e.value)).as_string();}, property_types::target_placement },
    property_meta_type{ "placement", enumeration_wrapper(MARKER_POINT_PLACEMENT), // FIXME - change property name
                        [](enumeration_wrapper e) { return enumeration<marker_placement_enum,marker_placement_enum_MAX>(marker_placement_enum(e.value)).as_string();}, property_types::target_markers_placement },
    property_meta_type{ "multi-policy", enumeration_wrapper(MARKER_EACH_MULTI),
                        [](enumeration_wrapper e) { return enumeration<marker_multi_policy_enum,marker_multi_policy_enum_MAX>(marker_multi_policy_enum(e.value)).as_string();}, property_types::target_markers_multipolicy },
    property_meta_type{ "placement", enumeration_wrapper(CENTROID_POINT_PLACEMENT), // FIXME - change property name
                        [](enumeration_wrapper e) { return enumeration<point_placement_enum,point_placement_enum_MAX>(point_placement_enum(e.value)).as_string();}, property_types::target_double },
    property_meta_type{ "colorizer", nullptr, nullptr, property_types::target_colorizer},
    property_meta_type{ "halo-transform", false, nullptr, property_types::target_transform },
    property_meta_type{ "num-columns", static_cast<value_integer>(0), nullptr, property_types::target_integer},
    property_meta_type{ "start-column", static_cast<value_integer>(1), nullptr, property_types::target_integer},
    property_meta_type{ "repeat-key", nullptr, nullptr, property_types::target_repeat_key},
    property_meta_type{ "symbolizer-properties", nullptr, nullptr, property_types::target_group_symbolizer_properties},
    property_meta_type{ "largest-box-only", false, nullptr, property_types::target_bool },
    property_meta_type{ "minimum-path-length", false, nullptr, property_types::target_double },
    property_meta_type{ "halo-comp-op", enumeration_wrapper(src_over),
                        [](enumeration_wrapper e) { return *comp_op_to_string(composite_mode_e(e.value)); }, property_types::target_halo_comp_op},
    property_meta_type{ "text-transform", enumeration_wrapper(NONE), [](enumeration_wrapper e)
                        {return enumeration<text_transform_enum,text_transform_enum_MAX>(text_transform_enum(e.value)).as_string();}, property_types::target_text_transform},
    property_meta_type{ "horizontal-alignment", enumeration_wrapper(H_LEFT), [](enumeration_wrapper e)
                        {return enumeration<horizontal_alignment_enum,horizontal_alignment_enum_MAX>(horizontal_alignment_enum(e.value)).as_string();},
                        property_types::target_horizontal_alignment},
    property_meta_type{ "justify-alignment", enumeration_wrapper(J_LEFT), [](enumeration_wrapper e)
                        {return enumeration<justify_alignment_enum,justify_alignment_enum_MAX>(justify_alignment_enum(e.value)).as_string();},
                        property_types::target_justify_alignment},
    property_meta_type{ "vertical-alignment", enumeration_wrapper(V_TOP), [](enumeration_wrapper e)
                        {return enumeration<vertical_alignment_enum,vertical_alignment_enum_MAX>(vertical_alignment_enum(e.value)).as_string();},
                        property_types::target_vertical_alignment},
    property_meta_type{ "upright", enumeration_wrapper(UPRIGHT_AUTO), [](enumeration_wrapper e)
                        {return enumeration<text_upright_enum,text_upright_enum_MAX>(text_upright_enum(e.value)).as_string();},
                        property_types::target_upright},
    property_meta_type{ "avoid-edges",false, nullptr, property_types::target_bool },

};

property_meta_type const& get_meta(mapnik::keys key)
{
    return key_meta[static_cast<int>(key)];
}

mapnik::keys get_key(std::string const& name)
{
    std::string name_copy(name);
    boost::algorithm::replace_all(name_copy,"_","-");
    for (unsigned i=0; i< const_max_key ; ++i)
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

}
