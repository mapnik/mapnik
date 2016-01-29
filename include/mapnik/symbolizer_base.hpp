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

#ifndef MAPNIK_SYMBOLIZER_BASE_HPP
#define MAPNIK_SYMBOLIZER_BASE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/path_expression.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/group/group_symbolizer_properties.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/text/font_feature_settings.hpp>
#include <mapnik/util/variant.hpp>

// stl
#include <memory>
#include <vector>
#include <iosfwd>
#include <map>

namespace agg { struct trans_affine; }

namespace mapnik
{

// fwd declares
// TODO - move these transform declares to own header
namespace detail { struct transform_node; }
using transform_list =  std::vector<detail::transform_node>;
using transform_list_ptr = std::shared_ptr<transform_list>;
using transform_type = transform_list_ptr;
class feature_impl;

MAPNIK_DECL void evaluate_transform(agg::trans_affine& tr,
                                    feature_impl const& feature,
                                    attributes const& vars,
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

using dash_array = std::vector<std::pair<double,double> >;

class text_placements;
using text_placements_ptr = std::shared_ptr<text_placements>;

namespace detail {

using value_base_type = util::variant<value_bool,
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
                                      group_symbolizer_properties_ptr,
                                      font_feature_settings>;

struct strict_value : value_base_type
{
    strict_value() = default;

    strict_value(const char* val)
        : value_base_type(val) {}

    template <typename T>
    strict_value(T const& obj)
        : value_base_type(typename detail::mapnik_value_type<T>::type(obj))
    {}

    template <typename T>
    strict_value(T && obj)
        noexcept(std::is_nothrow_constructible<value_base_type, T && >::value)
        : value_base_type(std::forward<T>(obj))
    {}
};

} // namespace detail

struct MAPNIK_DECL symbolizer_base
{
    using value_type = detail::strict_value;
    using key_type =  mapnik::keys;
    using cont_type = std::map<key_type, value_type>;
    cont_type properties;
};

inline bool is_expression(symbolizer_base::value_type const& val)
{
    return val.is<expression_ptr>();
}

inline bool operator==(symbolizer_base const& lhs, symbolizer_base const& rhs)
{
    return lhs.properties.size() == rhs.properties.size() &&
        std::equal(lhs.properties.begin(), lhs.properties.end(), rhs.properties.begin());
}

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
struct MAPNIK_DECL dot_symbolizer : public symbolizer_base {};

// symbolizer
using symbolizer = util::variant<point_symbolizer,
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
                                 debug_symbolizer,
                                 dot_symbolizer>;

}

#endif // MAPNIK_SYMBOLIZER_BASE_HPP
