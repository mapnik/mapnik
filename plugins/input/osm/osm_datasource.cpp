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

// stl
#include <stdexcept>
#include <set>

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/query.hpp>
#include <mapnik/boolean.hpp>

// boost

#include "osm_datasource.hpp"
#include "osm_featureset.hpp"
#include "dataset_deliverer.h"
#include "osmtagtypes.h"
#include "osmparser.h"

using mapnik::String;
using mapnik::Double;
using mapnik::Integer;
using mapnik::datasource_exception;
using mapnik::filter_in_box;
using mapnik::filter_at_point;
using mapnik::attribute_descriptor;

DATASOURCE_PLUGIN(osm_datasource)

osm_datasource::osm_datasource(const parameters& params)
    : datasource (params),
      extent_(),
      type_(datasource::Vector),
      desc_(osm_datasource::name(), *params.get<std::string>("encoding", "utf-8"))
{
    osm_data_ = nullptr;
    std::string osm_filename = *params.get<std::string>("file", "");
    std::string parser = *params.get<std::string>("parser", "libxml2");
    std::string url = *params.get<std::string>("url", "");
    std::string bbox = *params.get<std::string>("bbox", "");

    // load the data
    if (url != "" && bbox != "")
    {
        throw datasource_exception("Error loading from URL is no longer supported (removed in >= Mapnik 2.3.x");
    }
    else if (osm_filename != "")
    {
        // if we supplied a filename, load from file
        if ((osm_data_ = dataset_deliverer::load_from_file(osm_filename, parser)) == nullptr)
        {
            std::string s("OSM Plugin: Error loading from file '");
            s += osm_filename + "'";
            throw datasource_exception(s);
        }
    }
    else
    {
        throw datasource_exception("OSM Plugin: Neither 'file' nor 'url' and 'bbox' specified");
    }


    osm_tag_types tagtypes;
    tagtypes.add_type("maxspeed", mapnik::Integer);
    tagtypes.add_type("z_order", mapnik::Integer);

    osm_data_->rewind();

    // Need code to get the attributes of all the data
    std::set<std::string> keys = osm_data_->get_keys();

    // Add the attributes to the datasource descriptor - assume they are
    // all of type String
    for (auto const& key : keys)
    {
        desc_.add_descriptor(attribute_descriptor(key, tagtypes.get_type(key)));
    }
    // Get the bounds of the data and set extent_ accordingly
    bounds b = osm_data_->get_bounds();
    extent_ = box2d<double>(b.w,b.s,b.e,b.n);
}

osm_datasource::~osm_datasource()
{
    // Do not do as is now static variable and cleaned up at exit
    //delete osm_data_;
}

const char * osm_datasource::name()
{
    return "osm";
}

mapnik::datasource::datasource_t osm_datasource::type() const
{
    return type_;
}

layer_descriptor osm_datasource::get_descriptor() const
{
    return desc_;
}

featureset_ptr osm_datasource::features(const query& q) const
{
    filter_in_box filter(q.get_bbox());
    // so we need to filter osm features by bbox here...

    return std::make_shared<osm_featureset<filter_in_box> >(filter,
                                                              osm_data_,
                                                              q.property_names(),
                                                              desc_.get_encoding());
}

featureset_ptr osm_datasource::features_at_point(coord2d const& pt, double tol) const
{
    filter_at_point filter(pt);
    // collect all attribute names
    std::set<std::string> names;
    for (auto const& elem : desc_.get_descriptors())
    {
        names.insert(elem.get_name());
    }
    return std::make_shared<osm_featureset<filter_at_point> >(filter,
                                                              osm_data_,
                                                              names,
                                                              desc_.get_encoding());
}

box2d<double> osm_datasource::envelope() const
{
    return extent_;
}

boost::optional<mapnik::datasource_geometry_t> osm_datasource::get_geometry_type() const
{
    return boost::optional<mapnik::datasource_geometry_t>(mapnik::datasource_geometry_t::Collection);
}
