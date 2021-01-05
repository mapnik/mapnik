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

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/layer.hpp>

// stl
#include <string>

namespace mapnik
{

layer::layer(std::string const& name, std::string const& srs)
    : name_(name),
      srs_(srs),
      minimum_scale_denom_(0),
      maximum_scale_denom_(std::numeric_limits<double>::max()),
      active_(true),
      queryable_(false),
      clear_label_cache_(false),
      cache_features_(false),
      group_by_(),
      styles_(),
      ds_(),
      buffer_size_(),
      maximum_extent_() {}

layer::layer(layer const& rhs)
    : name_(rhs.name_),
      srs_(rhs.srs_),
      minimum_scale_denom_(rhs.minimum_scale_denom_),
      maximum_scale_denom_(rhs.maximum_scale_denom_),
      active_(rhs.active_),
      queryable_(rhs.queryable_),
      clear_label_cache_(rhs.clear_label_cache_),
      cache_features_(rhs.cache_features_),
      group_by_(rhs.group_by_),
      styles_(rhs.styles_),
      ds_(rhs.ds_),
      buffer_size_(rhs.buffer_size_),
      maximum_extent_(rhs.maximum_extent_) {}

layer::layer(layer && rhs)
    : name_(std::move(rhs.name_)),
      srs_(std::move(rhs.srs_)),
      minimum_scale_denom_(std::move(rhs.minimum_scale_denom_)),
      maximum_scale_denom_(std::move(rhs.maximum_scale_denom_)),
      active_(std::move(rhs.active_)),
      queryable_(std::move(rhs.queryable_)),
      clear_label_cache_(std::move(rhs.clear_label_cache_)),
      cache_features_(std::move(rhs.cache_features_)),
      group_by_(std::move(rhs.group_by_)),
      styles_(std::move(rhs.styles_)),
      ds_(std::move(rhs.ds_)),
      buffer_size_(std::move(rhs.buffer_size_)),
      maximum_extent_(std::move(rhs.maximum_extent_)) {}

layer& layer::operator=(layer rhs)
{
    using std::swap;
    std::swap(this->name_,rhs.name_);
    std::swap(this->srs_, rhs.srs_);
    std::swap(this->minimum_scale_denom_, rhs.minimum_scale_denom_);
    std::swap(this->maximum_scale_denom_,rhs.maximum_scale_denom_);
    std::swap(this->active_, rhs.active_);
    std::swap(this->queryable_, rhs.queryable_);
    std::swap(this->clear_label_cache_, rhs.clear_label_cache_);
    std::swap(this->cache_features_, rhs.cache_features_);
    std::swap(this->group_by_, rhs.group_by_);
    std::swap(this->styles_, rhs.styles_);
    std::swap(this->ds_, rhs.ds_);
    std::swap(this->buffer_size_, rhs.buffer_size_);
    std::swap(this->maximum_extent_, rhs.maximum_extent_);
    return *this;
}

bool layer::operator==(layer const& rhs) const
{
    return (name_ == rhs.name_) &&
        (srs_ == rhs.srs_) &&
        (minimum_scale_denom_ == rhs.minimum_scale_denom_) &&
        (maximum_scale_denom_ == rhs.maximum_scale_denom_) &&
        (active_ == rhs.active_) &&
        (queryable_ == rhs.queryable_) &&
        (clear_label_cache_ == rhs.clear_label_cache_) &&
        (cache_features_ == rhs.cache_features_) &&
        (group_by_ == rhs.group_by_) &&
        (styles_ == rhs.styles_) &&
        ((ds_ && rhs.ds_) ? *ds_ == *rhs.ds_ : ds_ == rhs.ds_) &&
        (buffer_size_ == rhs.buffer_size_) &&
        (maximum_extent_ == rhs.maximum_extent_);
}

layer::~layer() {}

void layer::set_name( std::string const& name)
{
    name_ = name;
}

std::string const& layer::name() const
{
    return name_;
}

void layer::set_srs(std::string const& srs)
{
    srs_ = srs;
}

std::string const& layer::srs() const
{
    return srs_;
}

void layer::add_style(std::string const& stylename)
{
    styles_.push_back(stylename);
}

std::vector<std::string> const& layer::styles() const
{
    return styles_;
}

std::vector<std::string> & layer::styles()
{
    return styles_;
}

void layer::set_minimum_scale_denominator(double minimum_scale_denom)
{
    minimum_scale_denom_=minimum_scale_denom;
}

void layer::set_maximum_scale_denominator(double maximum_scale_denom)
{
    maximum_scale_denom_=maximum_scale_denom;
}

double layer::minimum_scale_denominator() const
{
    return minimum_scale_denom_;
}

double layer::maximum_scale_denominator() const
{
    return maximum_scale_denom_;
}

void layer::set_active(bool active)
{
    active_=active;
}

bool layer::active() const
{
    return active_;
}

bool layer::visible(double scale_denom) const
{
    return active() && scale_denom >= minimum_scale_denom_ - 1e-6 && scale_denom < maximum_scale_denom_ + 1e-6;
}

void layer::set_queryable(bool queryable)
{
    queryable_=queryable;
}

bool layer::queryable() const
{
    return queryable_;
}

datasource_ptr layer::datasource() const
{
    return ds_;
}

void layer::set_datasource(datasource_ptr const& ds)
{
    ds_ = ds;
}

void layer::set_maximum_extent(box2d<double> const& box)
{
    maximum_extent_.reset(box);
}

boost::optional<box2d<double> > const& layer::maximum_extent() const
{
    return maximum_extent_;
}

void layer::reset_maximum_extent()
{
    maximum_extent_.reset();
}

void layer::set_buffer_size(int size)
{
    buffer_size_.reset(size);
}

boost::optional<int> const& layer::buffer_size() const
{
    return buffer_size_;
}

void layer::reset_buffer_size()
{
    buffer_size_.reset();
}

box2d<double> layer::envelope() const
{
    if (ds_) return ds_->envelope();
    return box2d<double>();
}

void layer::set_clear_label_cache(bool clear)
{
    clear_label_cache_ = clear;
}

bool layer::clear_label_cache() const
{
    return clear_label_cache_;
}

void layer::set_cache_features(bool cache_features)
{
    cache_features_ = cache_features;
}

bool layer::cache_features() const
{
    return cache_features_;
}

void layer::set_group_by(std::string const& column)
{
    group_by_ = column;
}

std::string const& layer::group_by() const
{
    return group_by_;
}

}
