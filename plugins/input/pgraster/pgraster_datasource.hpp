/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Sandro Santilli <strk@keybit.net>
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

#ifndef PGRASTER_DATASOURCE_HPP
#define PGRASTER_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/value_types.hpp>

// boost
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

// stl
#include <vector>
#include <string>

#include "connection_manager.hpp"
#include "resultset.hpp"
#include "cursorresultset.hpp"

using mapnik::transcoder;
using mapnik::datasource;
using mapnik::feature_style_context_map;
using mapnik::processor_context_ptr;
using mapnik::box2d;
using mapnik::layer_descriptor;
using mapnik::featureset_ptr;
using mapnik::feature_ptr;
using mapnik::query;
using mapnik::parameters;
using mapnik::coord2d;

typedef boost::shared_ptr< ConnectionManager::PoolType> CnxPool_ptr;

struct pgraster_overview
{
  std::string schema;
  std::string table;
  std::string column;
  float scale; // max absolute scale between x and y
};


class pgraster_datasource : public datasource
{
public:
    pgraster_datasource(const parameters &params);
    ~pgraster_datasource();
    mapnik::datasource::datasource_t type() const;
    static const char * name();
    processor_context_ptr get_context(feature_style_context_map &) const;
    featureset_ptr features_with_context(query const& q, processor_context_ptr ctx) const;
    featureset_ptr features(query const& q) const;
    featureset_ptr features_at_point(coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    boost::optional<mapnik::datasource::geometry_t> get_geometry_type() const;
    layer_descriptor get_descriptor() const;

private:
    std::string sql_bbox(box2d<double> const& env) const;
    std::string populate_tokens(std::string const& sql, double scale_denom, box2d<double> const& env, double pixel_width, double pixel_height) const;
    std::string populate_tokens(std::string const& sql) const;
    boost::shared_ptr<IResultSet> get_resultset(boost::shared_ptr<Connection> &conn, std::string const& sql, CnxPool_ptr const& pool, processor_context_ptr ctx= processor_context_ptr()) const;
    static const std::string RASTER_COLUMNS;
    static const std::string RASTER_OVERVIEWS;
    static const std::string SPATIAL_REF_SYS;
    static const double FMAX;

    const std::string uri_;
    const std::string username_;
    const std::string password_;
    // table name (schema qualified or not) or subquery
    const std::string table_;
    // schema name (possibly extracted from table_)
    std::string schema_;
    // table name (possibly extracted from table_)
    std::string raster_table_;
    const std::string raster_field_;
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
    const std::string bbox_token_;
    const std::string scale_denom_token_;
    const std::string pixel_width_token_;
    const std::string pixel_height_token_;
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
