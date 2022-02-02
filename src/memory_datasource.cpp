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
#include <mapnik/debug.hpp>
#include <mapnik/query.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/memory_datasource.hpp>
#include <mapnik/memory_featureset.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/geometry/envelope.hpp>

// stl
#include <algorithm>

using mapnik::datasource;
using mapnik::parameters;


DATASOURCE_PLUGIN_EXPORT(mapnik::memory_datasource_plugin)

namespace mapnik {

DATASOURCE_PLUGIN_IMPL(memory_datasource_plugin, memory_datasource);
DATASOURCE_PLUGIN_EMPTY_AFTER_LOAD(memory_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_BEFORE_UNLOAD(memory_datasource_plugin);

struct accumulate_extent
{
    accumulate_extent(box2d<double>& ext)
        : ext_(ext)
        , first_(true)
    {}

    void operator()(feature_ptr const& feat)
    {
        auto const& geom = feat->get_geometry();
        auto bbox = geometry::envelope(geom);
        if (first_)
        {
            first_ = false;
            ext_ = bbox;
        }
        else
        {
            ext_.expand_to_include(bbox);
        }
    }

    box2d<double>& ext_;
    bool first_;
};

const char* memory_datasource::name()
{
    return mapnik::memory_datasource_plugin::kName;
}

memory_datasource::memory_datasource(parameters const& _params)
    : datasource(_params)
    , desc_(memory_datasource::name(), *params_.get<std::string>("encoding", "utf-8"))
    , type_(datasource::Vector)
    , bbox_check_(*params_.get<boolean_type>("bbox_check", true))
    , type_set_(false)
{}

memory_datasource::~memory_datasource() {}

void memory_datasource::push(feature_ptr feature)
{
    // TODO - collect attribute descriptors?
    // desc_.add_descriptor(attribute_descriptor(fld_name,mapnik::Integer));
    if (feature->get_raster())
    {
        // if a feature has a raster_ptr set it must be of raster type.
        if (!type_set_)
        {
            type_ = datasource::Raster;
            type_set_ = true;
        }
        else if (type_ == datasource::Vector)
        {
            throw std::runtime_error("Can not add a raster feature to a memory datasource that contains vectors");
        }
    }
    else
    {
        if (!type_set_)
        {
            type_set_ = true;
        }
        else if (type_ == datasource::Raster)
        {
            throw std::runtime_error("Can not add a vector feature to a memory datasource that contains rasters");
        }
    }
    features_.push_back(feature);
    dirty_extent_ = true;
}

datasource::datasource_t memory_datasource::type() const
{
    return type_;
}

featureset_ptr memory_datasource::features(const query& q) const
{
    if (features_.empty())
    {
        return mapnik::make_invalid_featureset();
    }
    return std::make_shared<memory_featureset>(q.get_bbox(), *this, bbox_check_);
}

featureset_ptr memory_datasource::features_at_point(coord2d const& pt, double tol) const
{
    if (features_.empty())
    {
        return mapnik::make_invalid_featureset();
    }
    box2d<double> box = box2d<double>(pt.x, pt.y, pt.x, pt.y);
    box.pad(tol);
    MAPNIK_LOG_DEBUG(memory_datasource) << "memory_datasource: Box=" << box << ", Point x=" << pt.x << ",y=" << pt.y;
    return std::make_shared<memory_featureset>(box, *this);
}

void memory_datasource::set_envelope(box2d<double> const& box)
{
    extent_ = box;
    dirty_extent_ = false;
}

box2d<double> memory_datasource::envelope() const
{
    if (!extent_.valid() || dirty_extent_)
    {
        accumulate_extent func(extent_);
        std::for_each(features_.begin(), features_.end(), func);
        dirty_extent_ = false;
    }
    return extent_;
}

boost::optional<datasource_geometry_t> memory_datasource::get_geometry_type() const
{
    // TODO - detect this?
    return datasource_geometry_t::Collection;
}

layer_descriptor memory_datasource::get_descriptor() const
{
    return desc_;
}

size_t memory_datasource::size() const
{
    return features_.size();
}

void memory_datasource::clear()
{
    features_.clear();
}

} // namespace mapnik
