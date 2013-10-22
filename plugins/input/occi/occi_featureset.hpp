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

#ifndef OCCI_FEATURESET_HPP
#define OCCI_FEATURESET_HPP

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/unicode.hpp>

// boost

#include <memory>

// oci
#include "occi_types.hpp"

#include <vector>

class occi_featureset : public mapnik::Featureset
{
public:
    occi_featureset(oracle::occi::StatelessConnectionPool* pool,
                    oracle::occi::Connection* conn,
                    mapnik::context_ptr const& ctx,
                    std::string const& sqlstring,
                    std::string const& encoding,
                    bool use_connection_pool,
                    bool use_wkb,
                    unsigned prefetch_rows);
    virtual ~occi_featureset();
    mapnik::feature_ptr next();

private:
    void convert_geometry (SDOGeometry* geom, mapnik::feature_ptr feature);
    void convert_ordinates (mapnik::feature_ptr feature,
                            const mapnik::geometry_type::types& geom_type,
                            const std::vector<oracle::occi::Number>& elem_info,
                            const std::vector<oracle::occi::Number>& ordinates,
                            const int dimensions,
                            const bool is_single_geom,
                            const bool is_point_geom);
    void fill_geometry_type (mapnik::geometry_type* geom,
                             const int real_offset,
                             const int next_offset,
                             const std::vector<oracle::occi::Number>& ordinates,
                             const int dimensions,
                             const bool is_point_geom);

    occi_connection_ptr conn_;
    oracle::occi::ResultSet* rs_;
    const std::unique_ptr<mapnik::transcoder> tr_;
    mapnik::value_integer feature_id_;
    mapnik::context_ptr ctx_;
    bool use_wkb_;
    std::vector<char> buffer_;
};

#endif // OCCI_FEATURESET_HPP
