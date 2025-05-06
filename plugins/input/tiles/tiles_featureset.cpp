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

// mapnik
#include <mapnik/well_known_srs.hpp>
#include "tiles_source.hpp"
#include "tiles_featureset.hpp"
// boost
#include <boost/format.hpp>

tiles_featureset::tiles_featureset(std::shared_ptr<mapnik::tiles_source> source_ptr,
                                   mapnik::context_ptr const& ctx,
                                   const int zoom,
                                   mapnik::box2d<double> const& extent,
                                   std::string const& layer,
                                   std::unordered_map<std::string, std::string>& vector_tile_cache,
                                   std::size_t datasource_hash)
    : source_ptr_(source_ptr)
    , context_(ctx)
    , zoom_(zoom)
    , extent_(extent)
    , layer_(layer)
    , vector_tile_(nullptr)
    , vector_tile_cache_(vector_tile_cache)
    , datasource_hash_(datasource_hash)
{
    extent_.set_minx(extent_.minx() + 1e-6);
    extent_.set_maxx(extent_.maxx() - 1e-6);
    extent_.set_miny(extent_.miny() + 1e-6);
    extent_.set_maxy(extent_.maxy() - 1e-6);

    int tile_count = 1 << zoom;
    xmin_ =
      static_cast<int>((extent_.minx() + mapnik::EARTH_CIRCUMFERENCE / 2) * (tile_count / mapnik::EARTH_CIRCUMFERENCE));
    xmax_ =
      static_cast<int>((extent_.maxx() + mapnik::EARTH_CIRCUMFERENCE / 2) * (tile_count / mapnik::EARTH_CIRCUMFERENCE));
    ymin_ = static_cast<int>(((mapnik::EARTH_CIRCUMFERENCE / 2) - extent_.maxy()) *
                             (tile_count / mapnik::EARTH_CIRCUMFERENCE));
    ymax_ = static_cast<int>(((mapnik::EARTH_CIRCUMFERENCE / 2) - extent_.miny()) *
                             (tile_count / mapnik::EARTH_CIRCUMFERENCE));
    x_ = xmin_;
    y_ = ymin_;
    open_tile();
}

tiles_featureset::~tiles_featureset() {}

bool tiles_featureset::valid() const
{
    return vector_tile_.get() != nullptr;
}

mapnik::feature_ptr tiles_featureset::next_feature()
{
    mapnik::feature_ptr f = mapnik::feature_ptr();
    if (valid())
    {
        f = vector_tile_->next();
    }
    return f;
}

mapnik::feature_ptr tiles_featureset::next()
{
    // If current tile is processed completely, go forward to the next tile.
    // else step forward to the next feature
    mapnik::feature_ptr f = next_feature();
    if (f)
    {
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

bool tiles_featureset::next_tile()
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

bool tiles_featureset::open_tile()
{
    auto datasource_key = (boost::format("%1%-%2%-%3%-%4%") % datasource_hash_ % zoom_ % x_ % y_).str();
    auto itr = vector_tile_cache_.find(datasource_key);
    if (itr == vector_tile_cache_.end())
    {
        auto decompressed = source_ptr_->get_tile(zoom_, x_, y_);
        vector_tile_cache_.emplace(datasource_key, decompressed);
        vector_tile_.reset(new mvt_io(std::move(decompressed), context_, x_, y_, zoom_, layer_));
    }
    else
    {
        std::string buffer = itr->second;
        vector_tile_.reset(new mvt_io(std::move(buffer), context_, x_, y_, zoom_, layer_));
    }
    return true;
}
