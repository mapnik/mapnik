/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#include <mapnik/feature_type_style.hpp>

// boost
#include <boost/foreach.hpp>

namespace mapnik
{

static const char * filter_mode_strings[] = {
    "all",
    "first",
    ""
};

IMPLEMENT_ENUM( filter_mode_e, filter_mode_strings )


feature_type_style::feature_type_style()
: filter_mode_(FILTER_ALL),
    filters_(),
    direct_filters_(),
    scale_denom_validity_(-1),
    opacity_(1.0f)
{}

feature_type_style::feature_type_style(feature_type_style const& rhs, bool deep_copy)
    : filter_mode_(rhs.filter_mode_),
      filters_(rhs.filters_),
      direct_filters_(rhs.direct_filters_),
      comp_op_(rhs.comp_op_),
      scale_denom_validity_(-1),
      opacity_(rhs.opacity_)
{
    if (!deep_copy) {
        rules_ = rhs.rules_;
    } else {
        rules::const_iterator it  = rhs.rules_.begin(),
            end = rhs.rules_.end();
        for(; it != end; ++it) {
            rules_.push_back(rule(*it, deep_copy));
        }
    }
}

feature_type_style& feature_type_style::operator=(feature_type_style const& rhs)
{
    if (this == &rhs) return *this;
    rules_=rhs.rules_;   
    filters_ = rhs.filters_;
    direct_filters_ = rhs.direct_filters_;
    comp_op_ = rhs.comp_op_;
    scale_denom_validity_ = -1;
    opacity_= rhs.opacity_;
    return *this;
}

void feature_type_style::add_rule(rule const& rule)
{
    rules_.push_back(rule);
    scale_denom_validity_ = -1;
}

rules const& feature_type_style::get_rules() const
{
    return rules_;
}

rules& feature_type_style::get_rules_nonconst()
{
    return rules_;
}

bool feature_type_style::active(double scale_denom) const
{
    BOOST_FOREACH(rule const& r, rules_)
    {
        if (r.active(scale_denom))
        {
            return true;
        }
    }
    return false;
}

void feature_type_style::set_filter_mode(filter_mode_e mode)
{
    filter_mode_ = mode;
}

filter_mode_e feature_type_style::get_filter_mode() const
{
    return filter_mode_;
}

std::vector<filter::filter_type>&  feature_type_style::image_filters()
{
    return filters_;
}

std::vector<filter::filter_type> const&  feature_type_style::image_filters() const
{
    return filters_;
}

std::vector<filter::filter_type>&  feature_type_style::direct_image_filters()
{
    return direct_filters_;
}

std::vector<filter::filter_type> const&  feature_type_style::direct_image_filters() const
{
    return direct_filters_;
}

void feature_type_style::set_comp_op(composite_mode_e comp_op)
{
    comp_op_ = comp_op;
}

boost::optional<composite_mode_e> feature_type_style::comp_op() const
{
    return comp_op_;
}

void feature_type_style::set_opacity(float opacity)
{
    opacity_ = opacity;
}

float feature_type_style::get_opacity() const
{
    return opacity_;
}

void feature_type_style::update_rule_cache(double scale_denom)
{
    if_rules_.clear();
    else_rules_.clear();
    also_rules_.clear();

    BOOST_FOREACH(rule const& r, rules_)
    {
        if (r.active(scale_denom))
        {
            if (r.has_else_filter())
            {
                else_rules_.push_back(const_cast<rule*>(&r));
            }
            else if (r.has_also_filter())
            {
                also_rules_.push_back(const_cast<rule*>(&r));
            }
            else
            {
                if_rules_.push_back(const_cast<rule*>(&r));
            }
        }
    }

    scale_denom_validity_ = scale_denom;
}

rule_ptrs const& feature_type_style::get_if_rules(double scale_denom)
{
    if (scale_denom_validity_ != scale_denom)
    {
        update_rule_cache(scale_denom);
    }
    return if_rules_;
}

rule_ptrs const& feature_type_style::get_else_rules(double scale_denom)
{
    if (scale_denom_validity_ != scale_denom)
    {
        update_rule_cache(scale_denom);
    }
    return else_rules_;
}

rule_ptrs const& feature_type_style::get_also_rules(double scale_denom)
{
    if (scale_denom_validity_ != scale_denom)
    {
        update_rule_cache(scale_denom);
    }
    return also_rules_;
}

}
