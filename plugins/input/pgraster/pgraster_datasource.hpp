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
 *****************************************************************************
 *
 * Initially developed by Sandro Santilli <strk@keybit.net> for CartoDB
 *
 *****************************************************************************/

#ifndef PGRASTER_DATASOURCE_HPP
#define PGRASTER_DATASOURCE_HPP

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
#include <mapnik/datasource_plugin.hpp>

// boost
#include <boost/optional.hpp>

// stl
#include <regex>
#include <vector>
#include <string>
#include <memory>

#include "../postgis/connection_manager.hpp"
#include "../postgis/resultset.hpp"
#include "../postgis/cursorresultset.hpp"

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

typedef std::shared_ptr<ConnectionManager::PoolType> CnxPool_ptr;

struct pgraster_overview
{
    std::string schema;
    std::string table;
    std::string column;
    float scale; // max absolute scale between x and y
};

DATASOURCE_PLUGIN_DEF(pgraster_datasource_plugin, pgraster);
class pgraster_datasource : public datasource
{
  public:
    pgraster_datasource(const parameters& params);
    ~pgraster_datasource();
    mapnik::datasource::datasource_t type() const;
    static const char* name();
    processor_context_ptr get_context(feature_style_context_map&) const;
    featureset_ptr features_with_context(query const& q, processor_context_ptr ctx) const;
    featureset_ptr features(query const& q) const;
    featureset_ptr features_at_point(coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    boost::optional<mapnik::datasource_geometry_t> get_geometry_type() const;
    layer_descriptor get_descriptor() const;

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
    std::shared_ptr<IResultSet> get_resultset(std::shared_ptr<Connection>& conn,
                                              std::string const& sql,
                                              CnxPool_ptr const& pool,
                                              processor_context_ptr ctx = processor_context_ptr()) const;
    static const std::string RASTER_COLUMNS;
    static const std::string RASTER_OVERVIEWS;
    static const std::string SPATIAL_REF_SYS;

    const std::string uri_;
    const std::string username_;
    const std::string password_;
    // table name (schema qualified or not) or subquery
    const std::string table_;
    const std::string raster_table_; // possibly schema-qualified
    const std::string raster_field_;
    std::string parsed_schema_; // extracted from raster_table_ or table_
    std::string parsed_table_;  // extracted from raster_table_ or table_
    std::string key_field_;
    mapnik::value_integer cursor_fetch_size_;
    mapnik::value_integer row_limit_;
    std::string geometryColumn_;
    mapnik::datasource::datasource_t type_;
    int srid_;
    // 1-based index of band to extract from the raster
    // 0 means fetch all bands
    // any index also forces color interpretation off so that values
    // arrives untouched into the resulting mapnik raster, for threatment
    // by raster colorizer
    int band_;
    // Available overviews, ordered by max scale, ascending
    std::vector<pgraster_overview> overviews_;
    mutable bool extent_initialized_;
    mutable mapnik::box2d<double> extent_;
    bool prescale_rasters_;
    bool use_overviews_;
    bool clip_rasters_;
    layer_descriptor desc_;
    ConnectionCreator<Connection> creator_;
    std::regex re_tokens_;
    int pool_max_size_;
    bool persist_connection_;
    bool extent_from_subquery_;
    bool estimate_extent_;
    int max_async_connections_;
    bool asynchronous_request_;
    int intersect_min_scale_;
    int intersect_max_scale_;
};

#endif // PGRASTER_DATASOURCE_HPP
