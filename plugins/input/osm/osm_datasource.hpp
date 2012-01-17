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

#ifndef OSM_DATASOURCE_HPP
#define OSM_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/box2d.hpp>

#include "osm.h"

using mapnik::datasource;
using mapnik::parameters;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::coord2d;
using mapnik::box2d;

class osm_datasource : public datasource
{
public:
    osm_datasource(const parameters& params, bool bind = true);
    virtual ~osm_datasource();
    mapnik::datasource::datasource_t type() const;
    featureset_ptr features(const query& q) const;
    featureset_ptr features_at_point(coord2d const& pt) const;
    box2d<double> envelope() const;
    boost::optional<mapnik::datasource::geometry_t> get_geometry_type() const;
    layer_descriptor get_descriptor() const;
    static std::string name();
    void bind() const;
private:
    mutable box2d<double> extent_;
    mutable osm_dataset* osm_data_;
    mapnik::datasource::datasource_t type_;
    mutable layer_descriptor desc_;
    // no copying
    osm_datasource(const osm_datasource&);
    osm_datasource& operator=(const osm_datasource&);
};

#endif // OSM_DATASOURCE_HPP
