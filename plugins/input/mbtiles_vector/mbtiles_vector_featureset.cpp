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

#include "mbtiles_vector_featureset.hpp"
#include "vector_tile_compression.hpp"
#include <boost/format.hpp>
#include <math.h>

mbtiles_vector_featureset::mbtiles_vector_featureset(std::shared_ptr<sqlite_connection> database,
        mapnik::context_ptr const& ctx, const int zoom,
        mapnik::box2d<double> const& extent, const std::string & layer) :
               database_(database),
               context_(ctx),
               zoom_(zoom),
               extent_(extent),
               layer_(layer),
               vector_tile_(nullptr)
{
    int tile_count = 1 << zoom;
    constexpr double width = 2.0 * 6378137 * M_PI;
    xmin_ = static_cast<int>((extent_.minx() + width / 2) * (tile_count / width));
    xmax_ = static_cast<int>((extent_.maxx() + width / 2) * (tile_count / width));
    ymin_ = static_cast<int>(((width / 2) - extent_.maxy()) * (tile_count / width));
    ymax_ = static_cast<int>(((width / 2) - extent_.miny()) * (tile_count / width));
    std::cerr << "xmin: " << xmin_ << " xmax:" << xmax_ << std::endl;
    std::cerr << "ymin: " << ymin_ << " ymax:" << ymax_ << std::endl;
    x_ = xmin_;
    y_ = ymin_;
    open_tile();
}

mbtiles_vector_featureset::~mbtiles_vector_featureset() { }

bool mbtiles_vector_featureset::valid() const
{
    return vector_tile_.get() != nullptr;
}

mapnik::feature_ptr mbtiles_vector_featureset::next_feature()
{
    mapnik::feature_ptr f = mapnik::feature_ptr();
    if (valid()) {
        f = vector_tile_->next();
    }
    return f;
}

mapnik::feature_ptr mbtiles_vector_featureset::next()
{
    // If current tile is processed completely, go forward to the next tile.
    // else step forward to the next feature
    mapnik::feature_ptr f = next_feature();
    if (f) {
        return f;
    }
    while (next_tile() && open_tile() && valid())
    {
        f = next_feature();
        if (f)
        {
            return f;
        }
    }
    return mapnik::feature_ptr();
}

int mbtiles_vector_featureset::convert_y(const int y) const
{
    return (1 << zoom_) - 1 - y;
}

bool mbtiles_vector_featureset::next_tile()
{
    ++x_;
    if (x_ <= xmax_)
    {
        return true;
    }
    x_ = xmin_;
    ++y_;
    return y_ <= ymax_;
}

bool mbtiles_vector_featureset::open_tile()
{
    std::string sql = (boost::format("SELECT tile_data FROM tiles WHERE zoom_level = %1% AND tile_column = %2% AND tile_row = %3%") % zoom_ % x_ % convert_y(y_)).str();
    std::cerr << sql << std::endl;
    std::shared_ptr<sqlite_resultset> result (database_->execute_query(sql));
    int size = 0;
    char const* blob = nullptr;
    if (result->is_valid() && result->step_next() && result->column_type(0) == SQLITE_BLOB)
    {
        blob = result->column_blob(0, size);
    }
    else
    {
        return false;
    }
    if (mapnik::vector_tile_impl::is_gzip_compressed(blob, size) ||
        mapnik::vector_tile_impl::is_zlib_compressed(blob, size))
    {
        std::string decompressed;
        mapnik::vector_tile_impl::zlib_decompress(blob, size, decompressed);
        vector_tile_.reset(new mvt_io(std::move(decompressed), context_, x_, y_, zoom_, layer_));
    } else {
        vector_tile_.reset(new mvt_io(std::string(blob, size), context_, x_, y_, zoom_, layer_));
    }
    return true;
}
