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

#ifndef MAPNIK_SQLITE_DATASOURCE_HPP
#define MAPNIK_SQLITE_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/value_types.hpp>

// boost
#include <boost/optional.hpp>
#include <memory>

// stl
#include <vector>
#include <string>

// sqlite
#include "sqlite_connection.hpp"

class sqlite_datasource : public mapnik::datasource
{
public:
    sqlite_datasource(mapnik::parameters const& params);
    virtual ~sqlite_datasource ();
    datasource::datasource_t type() const;
    static const char * name();
    mapnik::featureset_ptr features(mapnik::query const& q) const;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    boost::optional<mapnik::datasource::geometry_t> get_geometry_type() const;
    mapnik::layer_descriptor get_descriptor() const;

private:
    // Fill init_statements with any statements
    // needed to attach auxillary databases
    void parse_attachdb(std::string const& attachdb) const;
    std::string populate_tokens(std::string const& sql) const;

    mapnik::box2d<double> extent_;
    bool extent_initialized_;
    mapnik::datasource::datasource_t type_;
    std::string dataset_name_;
    std::shared_ptr<sqlite_connection> dataset_;
    std::string table_;
    std::string fields_;
    std::string metadata_;
    std::string geometry_table_;
    std::string geometry_field_;
    std::string index_table_;
    std::string key_field_;
    int row_offset_;
    mapnik::value_integer row_limit_;
    // TODO - also add to postgis.input
    const std::string intersects_token_;
    mapnik::layer_descriptor desc_;
    mapnik::wkbFormat format_;
    bool use_spatial_index_;
    bool has_spatial_index_;
    bool using_subquery_;
    mutable std::vector<std::string> init_statements_;
};

#endif // MAPNIK_SQLITE_DATASOURCE_HPP
