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

#ifndef POSTGIS_DATASOURCE_HPP
#define POSTGIS_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>

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
using mapnik::box2d;
using mapnik::layer_descriptor;
using mapnik::featureset_ptr;
using mapnik::feature_ptr;
using mapnik::query;
using mapnik::parameters;
using mapnik::coord2d;

class postgis_datasource : public datasource
{
public:
    postgis_datasource(const parameters &params, bool bind=true);
    ~postgis_datasource();
    mapnik::datasource::datasource_t type() const;
    static const char * name();
    featureset_ptr features(const query& q) const;
    featureset_ptr features_at_point(coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    boost::optional<mapnik::datasource::geometry_t> get_geometry_type() const;
    layer_descriptor get_descriptor() const;
    void bind() const;

private:
    std::string sql_bbox(box2d<double> const& env) const;
    std::string populate_tokens(std::string const& sql, double scale_denom, box2d<double> const& env, double pixel_width, double pixel_height) const;
    std::string populate_tokens(std::string const& sql) const;
    static std::string unquote(std::string const& sql);
    boost::shared_ptr<IResultSet> get_resultset(boost::shared_ptr<Connection> const &conn, std::string const& sql) const;

    static const std::string GEOMETRY_COLUMNS;
    static const std::string SPATIAL_REF_SYS;
    static const double FMAX;

    const std::string uri_;
    const std::string username_;
    const std::string password_;
    const std::string table_;
    mutable std::string schema_;
    mutable std::string geometry_table_;
    const std::string geometry_field_;
    mutable std::string key_field_;
    const int cursor_fetch_size_;
    const int row_limit_;
    mutable std::string geometryColumn_;
    mapnik::datasource::datasource_t type_;
    mutable int srid_;
    mutable bool extent_initialized_;
    mutable mapnik::box2d<double> extent_;
    mutable bool simplify_geometries_;
    mutable layer_descriptor desc_;
    ConnectionCreator<Connection> creator_;
    const std::string bbox_token_;
    const std::string scale_denom_token_;
    const std::string pixel_width_token_;
    const std::string pixel_height_token_;
    bool persist_connection_;
    bool extent_from_subquery_;
    // params below are for testing purposes only (will likely be removed at any time)
    int intersect_min_scale_;
    int intersect_max_scale_;
};

#endif // POSTGIS_DATASOURCE_HPP
