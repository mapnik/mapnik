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

#include <mapnik/well_known_srs.hpp>
#include "pmtiles_featureset.hpp"
#include "vector_tile_compression.hpp"
#include "pmtiles_file.hpp"
#include <boost/format.hpp>

pmtiles_featureset::pmtiles_featureset(std::shared_ptr<mapnik::pmtiles_file> file_ptr,
                                       mapnik::context_ptr const& ctx, const int zoom,
                                       mapnik::box2d<double> const& extent, std::string const& layer,
                                       std::unordered_map<std::string, std::string> & vector_tile_cache,
                                       std::size_t datasource_hash)
:
    file_ptr_(file_ptr),
    context_(ctx),
    zoom_(zoom),
    extent_(extent),
    layer_(layer),
    vector_tile_(nullptr),
    vector_tile_cache_(vector_tile_cache),
    datasource_hash_(datasource_hash)
{
    int tile_count = 1 << zoom;
    xmin_ = static_cast<int>((extent_.minx() + mapnik::EARTH_CIRCUMFERENCE / 2) * (tile_count / mapnik::EARTH_CIRCUMFERENCE));
    xmax_ = static_cast<int>((extent_.maxx() + mapnik::EARTH_CIRCUMFERENCE / 2) * (tile_count / mapnik::EARTH_CIRCUMFERENCE));
    ymin_ = static_cast<int>(((mapnik::EARTH_CIRCUMFERENCE / 2) - extent_.maxy()) * (tile_count / mapnik::EARTH_CIRCUMFERENCE));
    ymax_ = static_cast<int>(((mapnik::EARTH_CIRCUMFERENCE / 2) - extent_.miny()) * (tile_count / mapnik::EARTH_CIRCUMFERENCE));
    x_ = xmin_;
    y_ = ymin_;
    open_tile();
}

pmtiles_featureset::~pmtiles_featureset() { }

bool pmtiles_featureset::valid() const
{
    return vector_tile_.get() != nullptr;
}

mapnik::feature_ptr pmtiles_featureset::next_feature()
{
    mapnik::feature_ptr f = mapnik::feature_ptr();
    if (valid()) {
        f = vector_tile_->next();
    }
    return f;
}

mapnik::feature_ptr pmtiles_featureset::next()
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

bool pmtiles_featureset::next_tile()
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

bool pmtiles_featureset::open_tile()
{
    auto tile = file_ptr_->get_tile(zoom_, x_, y_);
    auto datasource_key = (boost::format("%1%-%2%-%3%-%4%") % datasource_hash_ % zoom_ % x_ % y_).str();
    auto itr = vector_tile_cache_.find(datasource_key);
    if (itr == vector_tile_cache_.end())
    {
        std::cerr << "\e[41m" << layer_ << " - " << datasource_key << "\e[0m" << std::endl;
        if (mapnik::vector_tile_impl::is_gzip_compressed(file_ptr_->data() + tile.first, tile.second) ||
            mapnik::vector_tile_impl::is_zlib_compressed(file_ptr_->data() + tile.first, tile.second))
        {
            std::string decompressed;
            mapnik::vector_tile_impl::zlib_decompress(file_ptr_->data() + tile.first, tile.second, decompressed);
            vector_tile_cache_.emplace(datasource_key, decompressed);
            vector_tile_.reset(new mvt_io(std::move(decompressed), context_, x_, y_, zoom_, layer_));
        } else {
            vector_tile_.reset(new mvt_io(std::string(file_ptr_->data() + tile.first, tile.second), context_, x_, y_, zoom_, layer_));
        }
    }
    else
    {
        std::cerr << "\e[42m" << layer_ << " - " << datasource_key << "\e[0m" << std::endl;
        std::string tile = itr->second;
        vector_tile_.reset(new mvt_io(std::move(tile), context_, x_, y_, zoom_, layer_));
    }
    return true;
}
