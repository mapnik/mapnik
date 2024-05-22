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

#ifndef MAPNIK_SYMBOLIZER_HPP
#define MAPNIK_SYMBOLIZER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/image_scaling.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/simplify.hpp>
#include <mapnik/enumeration.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/path_expression.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/color.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/symbolizer_base.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/text/font_feature_settings.hpp>
#include <mapnik/util/dasharray_parser.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/symbolizer_default_values.hpp>
// stl
#include <type_traits>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <tuple>
#include <optional>

namespace mapnik {

// symbolizer properties target types
enum class property_types : std::uint8_t {
    target_bool = 1,
    target_double,
    target_integer,
    target_color,
    target_comp_op,
    target_line_cap,
    target_line_join,
    target_line_rasterizer,
    target_halo_rasterizer,
    target_point_placement,
    target_pattern_alignment,
    target_debug_symbolizer_mode,
    target_marker_placement,
    target_marker_multi_policy,
    target_string,
    target_transform,
    target_placement,
    target_dash_array,
    target_colorizer,
    target_repeat_key,
    target_group_symbolizer_properties,
    target_halo_comp_op,
    target_simplify_algorithm,
    target_markers_placement,
    target_markers_multipolicy,
    target_gamma_method,
    target_text_transform,
    target_horizontal_alignment,
    target_justify_alignment,
    target_vertical_alignment,
    target_upright,
    target_direction,
    target_font_feature_settings,
    target_line_pattern,
    target_smooth_algorithm,
    target_scaling_method
};

template<typename T>
struct evaluate_path_wrapper
{
    using result_type = T;
    template<typename T1, typename T2>
    result_type operator()(T1 const&, T2 const&) const
    {
        return result_type();
    }
};

template<>
struct evaluate_path_wrapper<std::string>
{
    template<typename T1, typename T2>
    std::string operator()(T1 const& expr, T2 const& feature) const
    {
        return mapnik::path_processor_type::evaluate(expr, feature);
    }
};

namespace detail {

template<typename T>
struct enum_traits
{};

template<>
struct enum_traits<composite_mode_e>
{
    using result_type = std::optional<composite_mode_e>;
    static result_type from_string(std::string const& str) { return comp_op_from_string(str); }
};

template<>
struct enum_traits<scaling_method_e>
{
    using result_type = std::optional<scaling_method_e>;
    static result_type from_string(std::string const& str) { return scaling_method_from_string(str); }
};

template<>
struct enum_traits<simplify_algorithm_e>
{
    using result_type = std::optional<simplify_algorithm_e>;
    static result_type from_string(std::string const& str) { return simplify_algorithm_from_string(str); }
};

#define ENUM_FROM_STRING(alias, e)                                                                                     \
    template<>                                                                                                         \
    struct enum_traits<e>                                                                                              \
    {                                                                                                                  \
        using result_type = std::optional<e>;                                                                          \
        static result_type from_string(std::string const& str)                                                         \
        {                                                                                                              \
            enumeration<e, alias##_to_string, alias##_from_string, alias##_lookup> enum_;                              \
            try                                                                                                        \
            {                                                                                                          \
                enum_.from_string(str);                                                                                \
                return result_type(e(enum_));                                                                          \
            }                                                                                                          \
            catch (...)                                                                                                \
            {                                                                                                          \
                return result_type();                                                                                  \
            }                                                                                                          \
        }                                                                                                              \
    };
ENUM_FROM_STRING(line_cap_e, line_cap_enum)
ENUM_FROM_STRING(line_join_e, line_join_enum)
ENUM_FROM_STRING(point_placement_e, point_placement_enum)
ENUM_FROM_STRING(line_rasterizer_e, line_rasterizer_enum)
ENUM_FROM_STRING(marker_placement_e, marker_placement_enum)
ENUM_FROM_STRING(marker_multi_policy_e, marker_multi_policy_enum)
ENUM_FROM_STRING(debug_symbolizer_mode_e, debug_symbolizer_mode_enum)
ENUM_FROM_STRING(pattern_alignment_e, pattern_alignment_enum)
ENUM_FROM_STRING(halo_rasterizer_e, halo_rasterizer_enum)
ENUM_FROM_STRING(label_placement_e, label_placement_enum)
ENUM_FROM_STRING(vertical_alignment_e, vertical_alignment_enum)
ENUM_FROM_STRING(horizontal_alignment_e, horizontal_alignment_enum)
ENUM_FROM_STRING(justify_alignment_e, justify_alignment_enum)
ENUM_FROM_STRING(text_transform_e, text_transform_enum)
ENUM_FROM_STRING(text_upright_e, text_upright_enum)
ENUM_FROM_STRING(direction_e, direction_enum)
ENUM_FROM_STRING(gamma_method_e, gamma_method_enum)
ENUM_FROM_STRING(line_pattern_e, line_pattern_enum)
ENUM_FROM_STRING(smooth_algorithm_e, smooth_algorithm_enum)

// enum
template<typename T, bool is_enum = true>
struct expression_result
{
    using result_type = T;
    static result_type convert(value_type const& val)
    {
        auto result = enum_traits<T>::from_string(val.to_string());
        if (result)
            return static_cast<result_type>(*result);
        return result_type(0);
    }
};

template<typename T>
struct expression_result<T, false>
{
    using result_type = T;
    static result_type convert(value_type const& val) { return val.convert<T>(); }
};

// enum
template<typename T, bool is_enum = true>
struct enumeration_result
{
    using result_type = T;
    static result_type convert(enumeration_wrapper const& e) { return static_cast<result_type>(e.value); }
};

template<typename T>
struct enumeration_result<T, false>
{
    using result_type = T;
    static result_type convert(enumeration_wrapper const&)
    {
        return result_type(); // FAIL
    }
};

// enum
template<typename T, bool is_enum = true>
struct put_impl
{
    static void apply(symbolizer_base& sym, keys key, T const& val)
    {
        auto itr = sym.properties.find(key);
        if (itr != sym.properties.end())
        {
            sym.properties[key] = enumeration_wrapper(val);
        }
        else
        {
            sym.properties.emplace(key, enumeration_wrapper(val));
        }
    }
};

template<typename T>
struct put_impl<T, false>
{
    static void apply(symbolizer_base& sym, keys key, T const& val)
    {
        auto itr = sym.properties.find(key);
        if (itr != sym.properties.end())
        {
            sym.properties[key] = val;
        }
        else
        {
            sym.properties.emplace(key, val);
        }
    }
};

} // namespace detail

template<typename T>
struct evaluate_expression_wrapper
{
    using result_type = T;

    template<typename T1, typename T2, typename T3>
    result_type operator()(T1 const& expr, T2 const& feature, T3 const& vars) const
    {
        mapnik::value_type result =
          util::apply_visitor(mapnik::evaluate<T2, mapnik::value_type, T3>(feature, vars), expr);
        return detail::expression_result<result_type, std::is_enum<result_type>::value>::convert(result);
    }
};

// mapnik::color
template<>
struct evaluate_expression_wrapper<mapnik::color>
{
    template<typename T1, typename T2, typename T3>
    mapnik::color operator()(T1 const& expr, T2 const& feature, T3 const& vars) const
    {
        mapnik::value_type val = util::apply_visitor(mapnik::evaluate<T2, mapnik::value_type, T3>(feature, vars), expr);
        if (val.is_null())
            return mapnik::color(0, 0, 0, 0); // transparent
        return mapnik::color(val.to_string());
    }
};

// enumeration wrapper
template<>
struct evaluate_expression_wrapper<mapnik::enumeration_wrapper>
{
    template<typename T1, typename T2, typename T3>
    mapnik::enumeration_wrapper operator()(T1 const& expr, T2 const& feature, T3 const& vars) const
    {
        mapnik::value_type val = util::apply_visitor(mapnik::evaluate<T2, mapnik::value_type, T3>(feature, vars), expr);
        return mapnik::enumeration_wrapper(val.to_int());
    }
};

template<>
struct evaluate_expression_wrapper<mapnik::dash_array>
{
    template<typename T1, typename T2, typename T3>
    mapnik::dash_array operator()(T1 const& expr, T2 const& feature, T3 const& vars) const
    {
        mapnik::value_type val = util::apply_visitor(mapnik::evaluate<T2, mapnik::value_type, T3>(feature, vars), expr);
        if (val.is_null())
            return dash_array();
        dash_array dash;
        std::string str = val.to_string();
        util::parse_dasharray(str, dash);
        return dash;
    }
};

// mapnik::font_feature_settings
template<>
struct evaluate_expression_wrapper<mapnik::font_feature_settings>
{
    template<typename T1, typename T2, typename T3>
    mapnik::font_feature_settings operator()(T1 const& expr, T2 const& feature, T3 const& vars) const
    {
        mapnik::value_type val = util::apply_visitor(mapnik::evaluate<T2, mapnik::value_type, T3>(feature, vars), expr);
        if (val.is_null())
            return mapnik::font_feature_settings();
        return mapnik::font_feature_settings(val.to_string());
    }
};

template<typename T>
struct extract_value
{
    using result_type = T;

    extract_value(mapnik::feature_impl const& feature, mapnik::attributes const& v)
        : feature_(feature)
        , vars_(v)
    {}

    auto operator()(mapnik::expression_ptr const& expr) const -> result_type
    {
        return evaluate_expression_wrapper<result_type>()(*expr, feature_, vars_);
    }

    auto operator()(mapnik::path_expression_ptr const& expr) const -> result_type
    {
        return evaluate_path_wrapper<result_type>()(*expr, feature_);
    }

    auto operator()(result_type const& val) const -> result_type { return val; }

    auto operator()(mapnik::enumeration_wrapper const& e) const -> result_type
    {
        return detail::enumeration_result<result_type, std::is_enum<result_type>::value>::convert(e);
    }

    template<typename T1>
    auto operator()(T1 const&) const -> result_type
    {
        return result_type();
    }

    mapnik::feature_impl const& feature_;
    mapnik::attributes const& vars_;
};

template<typename T1>
struct extract_raw_value
{
    using result_type = T1;

    auto operator()(result_type const& val) const -> result_type const& { return val; }

    auto operator()(mapnik::enumeration_wrapper const& e) const -> result_type
    {
        return detail::enumeration_result<result_type, std::is_enum<result_type>::value>::convert(e);
    }

    template<typename T2>
    auto operator()(T2 const&) const -> result_type
    {
        return result_type();
    }
};

using property_meta_type = std::tuple<const char*, std::function<std::string(enumeration_wrapper)>, property_types>;
MAPNIK_DECL property_meta_type const& get_meta(mapnik::keys key);
MAPNIK_DECL mapnik::keys get_key(std::string const& name);

template<typename T>
void put(symbolizer_base& sym, keys key, T const& val)
{
    constexpr bool enum_ = std::is_enum<T>::value;
    detail::put_impl<T, enum_>::apply(sym, key, val);
}

inline bool has_key(symbolizer_base const& sym, keys key)
{
    return (sym.properties.count(key) == 1);
}

template<typename T, keys key>
T get(symbolizer_base const& sym, mapnik::feature_impl const& feature, attributes const& vars)
{
    using const_iterator = symbolizer_base::cont_type::const_iterator;
    const_iterator itr = sym.properties.find(key);
    if (itr != sym.properties.end())
    {
        return util::apply_visitor(extract_value<T>(feature, vars), itr->second);
    }
    return mapnik::symbolizer_default<T, key>::value();
}

template<typename T>
T get(symbolizer_base const& sym,
      keys key,
      mapnik::feature_impl const& feature,
      attributes const& vars,
      T const& default_value)
{
    using const_iterator = symbolizer_base::cont_type::const_iterator;
    const_iterator itr = sym.properties.find(key);
    if (itr != sym.properties.end())
    {
        return util::apply_visitor(extract_value<T>(feature, vars), itr->second);
    }
    return default_value;
}

template<typename T>
std::optional<T>
  get_optional(symbolizer_base const& sym, keys key, mapnik::feature_impl const& feature, attributes const& vars)
{
    using const_iterator = symbolizer_base::cont_type::const_iterator;
    const_iterator itr = sym.properties.find(key);
    if (itr != sym.properties.end())
    {
        return util::apply_visitor(extract_value<T>(feature, vars), itr->second);
    }
    return std::nullopt;
}

template<typename T>
T get(symbolizer_base const& sym, keys key)
{
    using const_iterator = symbolizer_base::cont_type::const_iterator;
    const_iterator itr = sym.properties.find(key);
    if (itr != sym.properties.end())
    {
        return util::apply_visitor(extract_raw_value<T>(), itr->second);
    }
    return T{};
}

template<typename T>
T get(symbolizer_base const& sym, keys key, T const& default_value)
{
    using const_iterator = symbolizer_base::cont_type::const_iterator;
    const_iterator itr = sym.properties.find(key);
    if (itr != sym.properties.end())
    {
        return util::apply_visitor(extract_raw_value<T>(), itr->second);
    }
    return default_value;
}

template<typename T>
std::optional<T> get_optional(symbolizer_base const& sym, keys key)
{
    using const_iterator = symbolizer_base::cont_type::const_iterator;
    const_iterator itr = sym.properties.find(key);
    if (itr != sym.properties.end())
    {
        return util::apply_visitor(extract_raw_value<T>(), itr->second);
    }
    return std::nullopt;
}

} // namespace mapnik

#endif // MAPNIK_SYMBOLIZER_HPP
