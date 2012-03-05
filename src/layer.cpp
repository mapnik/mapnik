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

//$Id: layer.cpp 17 2005-03-08 23:58:43Z pavlenko $

// mapnik
#include <mapnik/layer.hpp>

#include <mapnik/datasource.hpp>
#include <mapnik/datasource_cache.hpp>

// stl
#include <string>
#include <iostream>


namespace mapnik
{
layer::layer(std::string const& name, std::string const& srs)
    : name_(name),
      srs_(srs),
      minZoom_(0),
      maxZoom_(std::numeric_limits<double>::max()),
      active_(true),
      queryable_(false),
      clear_label_cache_(false),
      cache_features_(false),
      group_by_(""),
      datasource_params_() {}

layer::layer(const layer& rhs)
    : name_(rhs.name_),
      srs_(rhs.srs_),
      minZoom_(rhs.minZoom_),
      maxZoom_(rhs.maxZoom_),
      active_(rhs.active_),
      queryable_(rhs.queryable_),
      clear_label_cache_(rhs.clear_label_cache_),
      cache_features_(rhs.cache_features_),
      group_by_(rhs.group_by_),
      styles_(rhs.styles_),
      datasource_params_(rhs.datasource_params_) {}

layer& layer::operator=(const layer& rhs)
{
    layer tmp(rhs);
    swap(tmp);
    return *this;
}

bool layer::operator==(layer const& other) const
{
    return (this == &other);
}

void layer::swap(const layer& rhs)
{
    name_=rhs.name_;
    srs_ = rhs.srs_;
    minZoom_=rhs.minZoom_;
    maxZoom_=rhs.maxZoom_;
    active_=rhs.active_;
    queryable_=rhs.queryable_;
    clear_label_cache_ = rhs.clear_label_cache_;
    cache_features_ = rhs.cache_features_;
    group_by_ = rhs.group_by_;
    styles_=rhs.styles_;
    datasource_params_ = rhs.datasource_params_;
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

void layer::setMinZoom(double minZoom)
{
    minZoom_=minZoom;
}

void layer::setMaxZoom(double maxZoom)
{
    maxZoom_=maxZoom;
}

double layer::getMinZoom() const
{
    return minZoom_;
}

double layer::getMaxZoom() const
{
    return maxZoom_;
}

void layer::setActive(bool active)
{
    active_=active;
}

bool layer::isActive() const
{
    return active_;
}

bool layer::isVisible(double scale) const
{
    return isActive() && scale >= minZoom_ - 1e-6 && scale < maxZoom_ + 1e-6;
}

void layer::setQueryable(bool queryable)
{
    queryable_=queryable;
}

bool layer::isQueryable() const
{
    return queryable_;
}

datasource_ptr layer::datasource() const
{
    if (ds_) return ds_;
    try
    {
        ds_ = datasource_cache::instance()->create(datasource_params_);
    }
    catch (const std::exception & ex )
    {
        throw config_error( ex.what() );
    }
    
    catch (...)
    {
        throw config_error("Unknown exception occured attempting to create datasoure for layer '" + name_ + "'");
    }
    
    return ds_;
}

void layer::set_datasource_parameters(parameters const& p)
{
    datasource_params_ = p;
}

parameters const& layer::datasource_parameters() const
{
    return datasource_params_;
}

box2d<double> layer::envelope() const
{
    datasource_ptr ds = this->datasource();
    if (ds) return ds->envelope();
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

void layer::set_group_by(std::string column)
{
    group_by_ = column;
}

std::string layer::group_by() const
{
    return group_by_;
}

}
