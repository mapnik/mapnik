// SPDX-License-Identifier: LGPL-2.1-or-later
/*****************************************************************************
 *
 * This file is part of Mapnik Vector Tile Plugin
 *
 * Copyright (C) 2023 Geofabrik GmbH
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

#ifndef MBTILES_VECTOR_FEATURESET_HPP_
#define MBTILES_VECTOR_FEATURESET_HPP_

#include <memory>
// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/datasource.hpp>
// sqlite
#include "sqlite_connection.hpp"
#include "mvt_io.hpp"

class mbtiles_vector_featureset : public mapnik::Featureset
{
public:
    mbtiles_vector_featureset(std::shared_ptr<sqlite_connection> database,
        mapnik::context_ptr const& ctx,
        const int zoom,
        mapnik::box2d<double> const& extent,
        const std::string & layer);

    virtual ~mbtiles_vector_featureset();
    mapnik::feature_ptr next();
private:
    mapnik::feature_ptr next_feature();
    bool valid() const;
    std::shared_ptr<sqlite_connection> database_;
    mapnik::context_ptr context_;
    int zoom_;
    mapnik::box2d<double> const& extent_;
    const std::string& layer_;
    std::unique_ptr<mvt_io> vector_tile_;
    int xmin_;
    int xmax_;
    int ymin_;
    int ymax_;
    /// x index of the currently accessed tile
    int x_ = 0;
    /// y index of the currently accessed tile
    int y_ = 0;
    /**
     * Transform Y index (tile_row) from z/x/y to TMS schema.
     */
    int convert_y(const int y) const;

    bool next_tile();
    bool open_tile();
};

#endif /* MBTILES_VECTOR_FEATURESET_HPP_ */
