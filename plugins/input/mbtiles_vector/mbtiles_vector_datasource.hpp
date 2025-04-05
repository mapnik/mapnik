// SPDX-License-Identifier: LGPL-2.1-or-later
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

#ifndef MBTILES_VECTOR_DATASOURCE_HPP_
#define MBTILES_VECTOR_DATASOURCE_HPP_

#include <mapnik/datasource.hpp>
//#include <mapnik/featureset.hpp>
#include "sqlite_connection.hpp"
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/datasource_plugin.hpp>

#include <memory>

#include <rapidjson/document.h>

#include <string>


DATASOURCE_PLUGIN_DEF(mbtiles_vector_datasource_plugin, mbtiles_vector);

class mbtiles_vector_datasource : public mapnik::datasource
{
public:
    mbtiles_vector_datasource(mapnik::parameters const& params);
    virtual ~mbtiles_vector_datasource ();
    mapnik::datasource::datasource_t type() const;
    static const char * name();
    mapnik::featureset_ptr features(mapnik::query const& q) const;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    std::optional<mapnik::datasource_geometry_t> get_geometry_type() const;
    mapnik::layer_descriptor get_descriptor() const;

private:
    void init(mapnik::parameters const& params);
    mapnik::context_ptr get_context_with_attributes() const;
    int zoom_from_string(const char* z);
    int zoom_from_string(const std::string& z);
    void parse_json();
    void raise_json_error(std::string message);
    void raise_json_parse_error(size_t pos, rapidjson::ParseErrorCode code);
    std::string database_path_;
    std::shared_ptr<sqlite_connection> dataset_;
    mapnik::box2d<double> extent_;
    int minzoom_ = 0;
    int maxzoom_ = 14;
    int zoom_ = 14;
    std::string json_;
    std::string layer_;
    mapnik::layer_descriptor desc_;
};



#endif /* MBTILES_VECTOR_DATASOURCE_HPP_ */
