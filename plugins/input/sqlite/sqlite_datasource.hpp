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

#ifndef MAPNIK_SQLITE_DATASOURCE_HPP
#define MAPNIK_SQLITE_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/datasource_plugin.hpp>

// stl
#include <vector>
#include <string>
#include <memory>

// sqlite
#include "sqlite_connection.hpp"

DATASOURCE_PLUGIN_DEF(sqlite_datasource_plugin, sqlite);

class sqlite_datasource : public mapnik::datasource
{
  public:
    sqlite_datasource(mapnik::parameters const& params);
    virtual ~sqlite_datasource();
    datasource::datasource_t type() const override;
    static char const* name();
    mapnik::featureset_ptr features(mapnik::query const& q) const override;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const override;
    mapnik::box2d<double> envelope() const override;
    std::optional<mapnik::datasource_geometry_t> get_geometry_type() const override;
    mapnik::layer_descriptor get_descriptor() const override;

  private:
    // Fill init_statements with any statements
    // needed to attach auxillary databases
    void parse_attachdb(std::string const& attachdb) const;
    std::string populate_tokens(std::string const& sql, double pixel_width, double pixel_height) const;

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
    std::string const intersects_token_;
    std::string const pixel_width_token_;
    std::string const pixel_height_token_;
    mapnik::layer_descriptor desc_;
    mapnik::wkbFormat format_;
    bool twkb_encoding_;
    bool use_spatial_index_;
    bool has_spatial_index_;
    bool using_subquery_;
    mutable std::vector<std::string> init_statements_;
};

#endif // MAPNIK_SQLITE_DATASOURCE_HPP
