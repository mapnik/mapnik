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

#include <mapnik/feature_type_style.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/enumeration.hpp>

// boost


namespace mapnik
{

static const char * filter_mode_strings[] = {
    "all",
    "first",
    ""
};

IMPLEMENT_ENUM( filter_mode_e, filter_mode_strings )


feature_type_style::feature_type_style()
    : rules_(),
      filter_mode_(FILTER_ALL),
      filters_(),
      direct_filters_(),
      comp_op_(),
      opacity_(1.0f),
      image_filters_inflate_(false)
{}

feature_type_style::feature_type_style(feature_type_style const& rhs)
    : rules_(rhs.rules_),
      filter_mode_(rhs.filter_mode_),
      filters_(rhs.filters_),
      direct_filters_(rhs.direct_filters_),
      comp_op_(rhs.comp_op_),
      opacity_(rhs.opacity_),
      image_filters_inflate_(rhs.image_filters_inflate_) {}

feature_type_style::feature_type_style(feature_type_style && rhs)
    : rules_(std::move(rhs.rules_)),
      filter_mode_(std::move(rhs.filter_mode_)),
      filters_(std::move(rhs.filters_)),
      direct_filters_(std::move(rhs.direct_filters_)),
      comp_op_(std::move(rhs.comp_op_)),
      opacity_(std::move(rhs.opacity_)),
      image_filters_inflate_(std::move(rhs.image_filters_inflate_)) {}

feature_type_style& feature_type_style::operator=(feature_type_style rhs)
{
    using std::swap;
    std::swap(this->rules_, rhs.rules_);
    std::swap(this->filter_mode_, rhs.filter_mode_);
    std::swap(this->filters_, rhs.filters_);
    std::swap(this->direct_filters_, rhs.direct_filters_);
    std::swap(this->comp_op_, rhs.comp_op_);
    std::swap(this->opacity_, rhs.opacity_);
    std::swap(this->image_filters_inflate_, rhs.image_filters_inflate_);
    return *this;
}

bool feature_type_style::operator==(feature_type_style const& rhs) const
{
    return (rules_ == rhs.rules_) &&
        (filter_mode_ == rhs.filter_mode_) &&
        (filters_ == rhs.filters_) &&
        (direct_filters_ == rhs.direct_filters_) &&
        (comp_op_ == rhs.comp_op_) &&
        (opacity_ == rhs.opacity_) &&
        (image_filters_inflate_ == rhs.image_filters_inflate_);
}

void feature_type_style::add_rule(rule && rule)
{
    rules_.push_back(std::move(rule));
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
    for (rule const& r : rules_)
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

void feature_type_style::set_image_filters_inflate(bool inflate)
{
    image_filters_inflate_ = inflate;
}

bool feature_type_style::image_filters_inflate() const
{
    return image_filters_inflate_;
}

}
