/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id$

#ifndef FEATURE_TYPE_STYLE_HPP
#define FEATURE_TYPE_STYLE_HPP
// mapnik
#include <mapnik/rule.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/enumeration.hpp>
// stl
#include <vector>

namespace mapnik
{

enum filter_mode_enum {
    FILTER_ALL,
    FILTER_FIRST,
    filter_mode_enum_MAX
};

DEFINE_ENUM( filter_mode_e, filter_mode_enum );

typedef std::vector<rule> rules;
typedef std::vector<rule*> rule_ptrs;
class feature_type_style
{
private:
    rules  rules_;
    filter_mode_e filter_mode_;

    // The rule_ptrs vectors are only valid for the scale_denom_validity_.
    double scale_denom_validity_;
    rule_ptrs if_rules_;
    rule_ptrs else_rules_;
    rule_ptrs also_rules_;
public:
    feature_type_style();

    feature_type_style(feature_type_style const& rhs);

    feature_type_style& operator=(feature_type_style const& rhs);

    void add_rule(rule const& rule);

    rules const& get_rules() const;
    rule_ptrs const& get_if_rules(double scale_denom);
    rule_ptrs const& get_else_rules(double scale_denom);
    rule_ptrs const& get_also_rules(double scale_denom);

    rules &get_rules_nonconst();

    void set_filter_mode(filter_mode_e mode);

    filter_mode_e get_filter_mode() const;

    ~feature_type_style() {}

private:
    void update_rule_cache(double scale_denom);
};
}

#endif //FEATURE_TYPE_STYLE_HPP
