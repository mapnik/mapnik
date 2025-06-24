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

#ifndef MAPNIK_FEATURE_TYPE_STYLE_HPP
#define MAPNIK_FEATURE_TYPE_STYLE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/enumeration.hpp>
#include <mapnik/image_filter_types.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/rule.hpp>

// stl
#include <vector>
#include <cstddef>
#include <optional>

namespace mapnik {

enum class filter_mode_enum { FILTER_ALL, FILTER_FIRST, filter_mode_enum_MAX };

DEFINE_ENUM(filter_mode_e, filter_mode_enum);

using rules = std::vector<rule>;

class MAPNIK_DECL feature_type_style
{
  private:
    rules rules_;
    filter_mode_e filter_mode_;
    // image_filters
    std::vector<filter::filter_type> filters_;
    std::vector<filter::filter_type> direct_filters_;
    // comp-op
    std::optional<composite_mode_e> comp_op_;
    float opacity_;
    bool image_filters_inflate_;
    friend void swap(feature_type_style& lhs, feature_type_style& rhs);

  public:
    // ctor
    feature_type_style();
    feature_type_style(feature_type_style const& rhs);
    feature_type_style(feature_type_style&& rhs);
    feature_type_style& operator=(feature_type_style rhs);

    // comparison
    bool operator==(feature_type_style const& rhs) const;

    void add_rule(rule&& rule);
    rules const& get_rules() const;
    rules& get_rules_nonconst();

    bool active(double scale_denom) const;

    void set_filter_mode(filter_mode_e mode);
    filter_mode_e get_filter_mode() const;

    // filters
    std::vector<filter::filter_type> const& image_filters() const;
    std::vector<filter::filter_type>& image_filters();
    std::vector<filter::filter_type> const& direct_image_filters() const;
    std::vector<filter::filter_type>& direct_image_filters();
    // compositing
    void set_comp_op(composite_mode_e comp_op);
    std::optional<composite_mode_e> comp_op() const;
    void set_opacity(float opacity);
    float get_opacity() const;
    void set_image_filters_inflate(bool inflate);
    bool image_filters_inflate() const;
    inline void reserve(std::size_t size) { rules_.reserve(size); }

    ~feature_type_style() {}
};
} // namespace mapnik

#endif // MAPNIK_FEATURE_TYPE_STYLE_HPP
