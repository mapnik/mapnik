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

#ifndef MAPNIK_SYMBOLIZER_HPP
#define MAPNIK_SYMBOLIZER_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/simplify.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/path_expression.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/color.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/text/placements/dummy.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/group/group_symbolizer_properties.hpp>
// stl
#include <type_traits>
#include <algorithm>
#include <memory>
#include <vector>
#include <string>
#include <functional>
// boost
#include <boost/variant/variant_fwd.hpp>

namespace agg { struct trans_affine; }

namespace mapnik
{

// fwd declares
// TODO - move these transform declares to own header
namespace detail { struct transform_node; }
typedef std::vector<detail::transform_node> transform_list;
typedef std::shared_ptr<transform_list>   transform_list_ptr;
typedef transform_list_ptr transform_type;
class feature_impl;

MAPNIK_DECL void evaluate_transform(agg::trans_affine& tr,
                                    feature_impl const& feature,
                                    transform_type const& trans_expr,
                                    double scale_factor=1.0);

struct enumeration_wrapper
{
    int value;
    enumeration_wrapper() = delete;
    template <typename T>
    explicit enumeration_wrapper(T value_)
        : value(value_) {}

    inline operator int() const
    {
        return value;
    }
};

typedef std::vector<std::pair<double,double> > dash_array;

struct  MAPNIK_DECL symbolizer_base
{
    typedef boost::variant<value_bool,
                           value_integer,
                           enumeration_wrapper,
                           value_double,
                           std::string,
                           color,
                           expression_ptr,
                           path_expression_ptr,
                           transform_type,
                           text_placements_ptr,
                           dash_array,
                           raster_colorizer_ptr,
                           group_symbolizer_properties_ptr> value_type;
    typedef mapnik::keys key_type;
    typedef std::map<key_type, value_type> cont_type;
    cont_type properties;
};

// symbolizer properties target types
enum class property_types : std::uint8_t
{
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
    target_group_symbolizer_properties
};

inline bool operator==(symbolizer_base const& lhs, symbolizer_base const& rhs)
{
    return lhs.properties.size() == rhs.properties.size() &&
        std::equal(lhs.properties.begin(), lhs.properties.end(), rhs.properties.begin());
}

template <typename T>
struct evaluate_path_wrapper
{
    typedef T result_type;
    template <typename T1, typename T2>
    result_type operator() (T1 const& expr, T2 const& feature) const
    {
        return result_type();
    }

};

template <>
struct evaluate_path_wrapper<std::string>
{
    template <typename T1, typename T2>
    std::string operator() (T1 const& expr, T2 const& feature) const
    {
        return mapnik::path_processor_type::evaluate(expr, feature);
    }
};

namespace detail {

// enum
template <typename T, bool is_enum = true>
struct expression_result
{
    typedef T result_type;
    static result_type convert(value_type const& val)
    {
        return static_cast<T>(val.convert<value_integer>());
    }
};

template <typename T>
struct expression_result<T,false>
{
    typedef T result_type;
    static result_type convert(value_type const& val)
    {
        return val.convert<T>();
    }
};

// enum
template <typename T, bool is_enum = true>
struct enumeration_result
{
    typedef T result_type;
    static result_type convert(enumeration_wrapper const& e)
    {
        return static_cast<result_type>(e.value);
    }
};

template <typename T>
struct enumeration_result<T,false>
{
    typedef T result_type;
    static result_type convert(enumeration_wrapper const& e)
    {
        return result_type();// FAIL
    }
};

// enum
template <typename T, bool is_enum = true >
struct put_impl
{
    static void apply(symbolizer_base & sym, keys key, T const& val)
    {
        auto itr = sym.properties.find(key);
        if (itr != sym.properties.end())
        {
            sym.properties[key] = enumeration_wrapper(val);
        }
        else
        {
            // NOTE: we use insert here instead of emplace
            // because of lacking std::map emplace support in libstdc++
            // http://gcc.gnu.org/bugzilla/show_bug.cgi?id=44436
            sym.properties.insert(std::make_pair(key, enumeration_wrapper(val)));
        }
    }
};

template <typename T>
struct put_impl<T, false>
{
    static void apply(symbolizer_base & sym, keys key, T const& val)
    {
        auto itr = sym.properties.find(key);
        if (itr != sym.properties.end())
        {
            sym.properties[key] = val;
        }
        else
        {
            sym.properties.insert(std::make_pair(key, val));
        }
    }
};

}

template <typename T>
struct evaluate_expression_wrapper
{
    typedef T result_type;

    template <typename T1, typename T2>
    result_type operator() (T1 const& expr, T2 const& feature) const
    {
        mapnik::value_type result = boost::apply_visitor(mapnik::evaluate<mapnik::feature_impl,mapnik::value_type>(feature), expr);
        return detail::expression_result<result_type, std::is_enum<result_type>::value>::convert(result);
    }
};

// mapnik::color
template <>
struct evaluate_expression_wrapper<mapnik::color>
{
    template <typename T1, typename T2>
    mapnik::color operator() (T1 const& expr, T2 const& feature) const
    {
        mapnik::value_type val = boost::apply_visitor(mapnik::evaluate<mapnik::feature_impl,mapnik::value_type>(feature), expr);
        // FIXME - throw instead?
        if (val.is_null()) return mapnik::color(255,192,203); // pink
        return mapnik::color(val.to_string());
    }
};

// enumeration wrapper
template <>
struct evaluate_expression_wrapper<mapnik::enumeration_wrapper>
{
    template <typename T1, typename T2>
    mapnik::enumeration_wrapper operator() (T1 const& expr, T2 const& feature) const
    {
        mapnik::value_type val = boost::apply_visitor(mapnik::evaluate<mapnik::feature_impl,mapnik::value_type>(feature), expr);
        return mapnik::enumeration_wrapper(val.to_int());
    }
};


template <typename T>
struct extract_value : public boost::static_visitor<T>
{
    typedef T result_type;

    extract_value(mapnik::feature_impl const& feature)
        : feature_(feature) {}

    auto operator() (mapnik::expression_ptr const& expr) const -> result_type
    {
        return evaluate_expression_wrapper<result_type>()(*expr,feature_);
    }

    auto operator() (mapnik::path_expression_ptr const& expr) const -> result_type
    {
        return evaluate_path_wrapper<result_type>()(*expr, feature_);
    }

    auto operator() (result_type const& val) const -> result_type
    {
        return val;
    }

    auto operator() (mapnik::enumeration_wrapper const& e) const -> result_type
    {
        return detail::enumeration_result<result_type, std::is_enum<result_type>::value>::convert(e);
    }

    template <typename T1>
    auto operator() (T1 const& val) const -> result_type
    {
        return result_type();
    }

    mapnik::feature_impl const& feature_;
};

template <typename T1>
struct extract_raw_value : public boost::static_visitor<T1>
{
    typedef T1 result_type;

    auto operator() (result_type const& val) const -> result_type const&
    {
        return val;
    }

    auto operator() (mapnik::enumeration_wrapper const& e) const -> result_type
    {
        return detail::enumeration_result<result_type, std::is_enum<result_type>::value>::convert(e);
    }

    template <typename T2>
    auto operator() (T2 const& val) const -> result_type
    {
        return result_type();
    }
};

template <typename T>
void put(symbolizer_base & sym, keys key, T const& val)
{
    constexpr bool enum_ = std::is_enum<T>::value;
    detail::put_impl<T, enum_ >::apply(sym, key, val);
}

template <typename T>
bool has_key(symbolizer_base const& sym, keys key)
{
    return (sym.properties.count(key) == 1);
}

template <typename T>
T get(symbolizer_base const& sym, keys key, mapnik::feature_impl const& feature, T const& _default_value = T())
{
    typedef symbolizer_base::cont_type::const_iterator const_iterator;
    const_iterator itr = sym.properties.find(key);
    if (itr != sym.properties.end())
    {
        return boost::apply_visitor(extract_value<T>(feature), itr->second);
    }
    return _default_value;
}

template <typename T>
boost::optional<T> get_optional(symbolizer_base const& sym, keys key, mapnik::feature_impl const& feature)
{
    typedef symbolizer_base::cont_type::const_iterator const_iterator;
    const_iterator itr = sym.properties.find(key);
    if (itr != sym.properties.end())
    {
        return boost::apply_visitor(extract_value<T>(feature), itr->second);
    }
    return boost::optional<T>();
}

template <typename T>
T get(symbolizer_base const& sym, keys key, T const& _default_value = T())
{
    typedef symbolizer_base::cont_type::const_iterator const_iterator;
    const_iterator itr = sym.properties.find(key);
    if (itr != sym.properties.end())
    {
        return boost::apply_visitor(extract_raw_value<T>(), itr->second);
    }
    return _default_value;
}

template <typename T>
boost::optional<T> get_optional(symbolizer_base const& sym, keys key)
{
    typedef symbolizer_base::cont_type::const_iterator const_iterator;
    const_iterator itr = sym.properties.find(key);
    if (itr != sym.properties.end())
    {
        return boost::apply_visitor(extract_raw_value<T>(), itr->second);
    }
    return boost::optional<T>();
}

template<typename Enum>
constexpr auto to_integral(Enum e) -> typename std::underlying_type<Enum>::type
{
    return static_cast<typename std::underlying_type<Enum>::type>(e);
}

typedef std::tuple<const char*, mapnik::symbolizer_base::value_type, std::function<std::string(enumeration_wrapper)>, property_types> property_meta_type;
property_meta_type const& get_meta(mapnik::keys key);
mapnik::keys get_key(std::string const& name);

// concrete symbolizer types
struct MAPNIK_DECL point_symbolizer : public symbolizer_base {};
struct MAPNIK_DECL line_symbolizer : public symbolizer_base {};
struct MAPNIK_DECL polygon_symbolizer : public symbolizer_base {};
struct MAPNIK_DECL text_symbolizer : public symbolizer_base {};
struct MAPNIK_DECL shield_symbolizer : public text_symbolizer {};
struct MAPNIK_DECL line_pattern_symbolizer : public symbolizer_base {};
struct MAPNIK_DECL polygon_pattern_symbolizer : public symbolizer_base {};
struct MAPNIK_DECL markers_symbolizer : public symbolizer_base {};
struct MAPNIK_DECL raster_symbolizer : public symbolizer_base {};
struct MAPNIK_DECL building_symbolizer : public symbolizer_base {};
struct MAPNIK_DECL group_symbolizer : public symbolizer_base {};
struct MAPNIK_DECL debug_symbolizer : public symbolizer_base {};

// symbolizer
typedef boost::variant<point_symbolizer,
                       line_symbolizer,
                       line_pattern_symbolizer,
                       polygon_symbolizer,
                       polygon_pattern_symbolizer,
                       raster_symbolizer,
                       shield_symbolizer,
                       text_symbolizer,
                       building_symbolizer,
                       markers_symbolizer,
                       group_symbolizer,
                       debug_symbolizer> symbolizer;


typedef std::vector<std::pair<double,double> > dash_array;

enum line_cap_enum
{
    BUTT_CAP,
    SQUARE_CAP,
    ROUND_CAP,
    line_cap_enum_MAX
};

DEFINE_ENUM( line_cap_e, line_cap_enum );

enum line_join_enum
{
    MITER_JOIN,
    MITER_REVERT_JOIN,
    ROUND_JOIN,
    BEVEL_JOIN,
    line_join_enum_MAX
};

DEFINE_ENUM( line_join_e, line_join_enum );

enum line_rasterizer_enum
{
    RASTERIZER_FULL,           // agg::renderer_scanline_aa_solid
    RASTERIZER_FAST,           // agg::rasterizer_outline_aa, twice as fast but only good for thin lines
    line_rasterizer_enum_MAX
};

DEFINE_ENUM( line_rasterizer_e, line_rasterizer_enum );


enum halo_rasterizer_enum
{
    HALO_RASTERIZER_FULL,
    HALO_RASTERIZER_FAST,
    halo_rasterizer_enum_MAX
};

DEFINE_ENUM(halo_rasterizer_e, halo_rasterizer_enum);

enum point_placement_enum
{
    CENTROID_POINT_PLACEMENT,
    INTERIOR_POINT_PLACEMENT,
    point_placement_enum_MAX
};

DEFINE_ENUM( point_placement_e, point_placement_enum );

enum pattern_alignment_enum
{
    LOCAL_ALIGNMENT,
    GLOBAL_ALIGNMENT,
    pattern_alignment_enum_MAX
};

DEFINE_ENUM( pattern_alignment_e, pattern_alignment_enum );

enum debug_symbolizer_mode_enum
{
    DEBUG_SYM_MODE_COLLISION,
    DEBUG_SYM_MODE_VERTEX,
    debug_symbolizer_mode_enum_MAX
};

DEFINE_ENUM( debug_symbolizer_mode_e, debug_symbolizer_mode_enum );


// markers
// TODO - consider merging with text_symbolizer label_placement_e
enum marker_placement_enum
{
    MARKER_POINT_PLACEMENT,
    MARKER_INTERIOR_PLACEMENT,
    MARKER_LINE_PLACEMENT,
    marker_placement_enum_MAX
};

DEFINE_ENUM( marker_placement_e, marker_placement_enum );

enum marker_multi_policy_enum
{
    MARKER_EACH_MULTI, // each component in a multi gets its marker
    MARKER_WHOLE_MULTI, // consider all components of a multi as a whole
    MARKER_LARGEST_MULTI, // only the largest component of a multi gets a marker
    marker_multi_policy_enum_MAX
};

DEFINE_ENUM( marker_multi_policy_e, marker_multi_policy_enum );

}

#endif // MAPNIK_SYMBOLIZER_HPP
