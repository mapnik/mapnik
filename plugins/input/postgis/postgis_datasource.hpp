/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#ifndef POSTGIS_DATASOURCE_HPP
#define POSTGIS_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/datasource_plugin.hpp>

// boost
#include <boost/optional.hpp>

// stl
#include <memory>
#include <regex>
#include <vector>
#include <string>

#include "connection_manager.hpp"
#include "resultset.hpp"
#include "cursorresultset.hpp"

using mapnik::box2d;
using mapnik::coord2d;
using mapnik::datasource;
using mapnik::feature_ptr;
using mapnik::feature_style_context_map;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::parameters;
using mapnik::processor_context_ptr;
using mapnik::query;
using mapnik::transcoder;

using CnxPool_ptr = std::shared_ptr<ConnectionManager::PoolType>;

DATASOURCE_PLUGIN_DEF(postgis_datasource_plugin, postgis);
class postgis_datasource : public datasource
{
  public:
    postgis_datasource(const parameters& params);
    ~postgis_datasource();
    mapnik::datasource::datasource_t type() const override;
    static const char* name();
    processor_context_ptr get_context(feature_style_context_map&) const override;
    featureset_ptr features_with_context(query const& q, processor_context_ptr ctx) const override;
    featureset_ptr features(query const& q) const override;
    featureset_ptr features_at_point(coord2d const& pt, double tol = 0) const override;
    mapnik::box2d<double> envelope() const override;
    std::optional<mapnik::datasource_geometry_t> get_geometry_type() const override;
    layer_descriptor get_descriptor() const override;

  private:
    std::string sql_bbox(box2d<double> const& env) const;
    std::string populate_tokens(std::string const& sql,
                                double scale_denom,
                                box2d<double> const& env,
                                double pixel_width,
                                double pixel_height,
                                mapnik::attributes const& vars,
                                bool intersect = true) const;
    std::string populate_tokens(std::string const& sql) const;
    void append_geometry_table(std::ostream& os) const;
    std::shared_ptr<IResultSet> get_resultset(std::shared_ptr<Connection>& conn,
                                              std::string const& sql,
                                              CnxPool_ptr const& pool,
                                              processor_context_ptr ctx = processor_context_ptr()) const;
    static const std::string GEOMETRY_COLUMNS;
    static const std::string SPATIAL_REF_SYS;

    const std::string uri_;
    const std::string username_;
    const std::string password_;
    const std::string table_;
    const std::string geometry_table_;
    const std::string geometry_field_;
    std::string parsed_schema_;
    std::string parsed_table_;
    std::string key_field_;
    mapnik::value_integer cursor_fetch_size_;
    mapnik::value_integer row_limit_;
    std::string geometryColumn_;
    mapnik::datasource::datasource_t type_;
    int srid_;
    mutable bool extent_initialized_;
    mutable mapnik::box2d<double> extent_;
    bool simplify_geometries_;
    layer_descriptor desc_;
    ConnectionCreator<Connection> creator_;
    int pool_max_size_;
    bool persist_connection_;
    bool extent_from_subquery_;
    bool estimate_extent_;
    int max_async_connections_;
    bool asynchronous_request_;
    bool twkb_encoding_;
    mapnik::value_double twkb_rounding_adjustment_;
    mapnik::value_double simplify_snap_ratio_;
    mapnik::value_double simplify_dp_ratio_;
    bool simplify_dp_preserve_;
    mapnik::value_double simplify_clip_resolution_;
    std::regex re_tokens_;
    int intersect_min_scale_;
    int intersect_max_scale_;
    bool key_field_as_attribute_;
};

#endif // POSTGIS_DATASOURCE_HPP
