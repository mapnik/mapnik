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
#include "xyz_featureset.hpp"
#include "vector_tile_compression.hpp"
// boost
#include <boost/format.hpp>

xyz_featureset::xyz_featureset(std::string url_template,
                               mapnik::context_ptr const& ctx,
                               int const zoom,
                               mapnik::box2d<double> const& extent,
                               std::string const& layer,
                               std::unordered_map<std::string, std::string>& vector_tile_cache,
                               std::size_t datasource_hash)
    : url_template_(url_template),
      context_(ctx),
      zoom_(zoom),
      extent_(extent),
      layer_(layer),
      vector_tile_(nullptr),
      vector_tile_cache_(vector_tile_cache),
      stash_(ioc_,targets_, queue_),
      datasource_hash_(datasource_hash)
{
    extent_.set_minx(extent_.minx() + 1e-6);
    extent_.set_maxx(extent_.maxx() - 1e-6);
    extent_.set_miny(extent_.miny() + 1e-6);
    extent_.set_maxy(extent_.maxy() - 1e-6);

    int tile_count = 1 << zoom;
    xmin_ = static_cast<int>((extent_.minx() + mapnik::EARTH_CIRCUMFERENCE / 2) *
                             (tile_count / mapnik::EARTH_CIRCUMFERENCE));
    xmax_ = static_cast<int>((extent_.maxx() + mapnik::EARTH_CIRCUMFERENCE / 2) *
                             (tile_count / mapnik::EARTH_CIRCUMFERENCE));
    ymin_ = static_cast<int>(((mapnik::EARTH_CIRCUMFERENCE / 2) - extent_.maxy()) *
                             (tile_count / mapnik::EARTH_CIRCUMFERENCE));
    ymax_ = static_cast<int>(((mapnik::EARTH_CIRCUMFERENCE / 2) - extent_.miny()) *
                             (tile_count / mapnik::EARTH_CIRCUMFERENCE));
    //std::cerr << "EXTENT:" << extent_ << std::endl;
    boost::urls::url url = boost::urls::format(url_template_, {{"z", zoom_}, {"x", 0}, {"y", 0}});
    //std::string scheme = url.scheme();
    host_ = url.host();
    port_ = url.port();
    //std::string path = url.path();
    std::cerr << "\e[31mxyz_featureset::xyz_featureset " << extent_ << " "
              << xmin_ << ":"  << xmax_ << " " << ymin_ << ":" << ymax_ <<  "\e[0m" << std::endl;


    //std::cerr << " NUM TILES: " << num_tiles_ << " TARGETS SIZE:" << stash_.targets().size()<< std::endl;
    //open_tile();
}

// xyz_featureset::init()
// {

// }

xyz_featureset::~xyz_featureset()
{
    //std::cerr << " xyz_featureset::~xyz_featureset() URL template:" << url_template_ << std::endl;
    for(std::size_t i = 0; i < workers_.size(); ++i)
    {
        workers_[i].join();
    }

}

bool xyz_featureset::valid() const
{
    return vector_tile_.get() != nullptr;
}

mapnik::feature_ptr xyz_featureset::next_feature()
{
    mapnik::feature_ptr f = mapnik::feature_ptr();
    if (valid())
    {
        f = vector_tile_->next();
    }
    return f;
}

mapnik::feature_ptr xyz_featureset::next()
{
    mapnik::feature_ptr f = next_feature();
    if (f)
    {
        return f;
    }
    while (next_tile() && valid())
    {
        f = next_feature();
        if (f)
        {
            return f;
        }
    }
    return mapnik::feature_ptr();
}

bool xyz_featureset::next_tile()
{
    if (first_)
    {
        first_ = false;
        for (int x = xmin_; x <= xmax_; ++x)
        {
            for (int y = ymin_; y <= ymax_; ++y)
            {
                ++num_tiles_;
                auto datasource_key = (boost::format("%1%-%2%-%3%-%4%") % datasource_hash_ % zoom_ % x % y).str();
                auto itr = vector_tile_cache_.find(datasource_key);
                if (itr == vector_tile_cache_.end())
                {
                    stash_.targets().emplace_back(zoom_, x, y);
                }
                else
                {
                    std::string buffer = itr->second;
                    stash_.push(tile_data(zoom_, x, y, std::move(buffer)));
                }
            }
        }
        if (!stash_.targets().empty())
        {
            std::size_t threads = 4;
            workers_.reserve(threads + 1);
            for(std::size_t i = 0; i < threads; ++i)
            {
                auto reporting_work = boost::asio::require(
                    ioc_.get_executor(),
                    boost::asio::execution::outstanding_work.tracked);

                workers_.emplace_back([this, reporting_work] {
                    boost::asio::io_context ioc;
                    std::make_shared<worker>(ioc, url_template_, stash_)->run(host_, port_);
                    ioc.run();
                });
            }
            workers_.emplace_back([this]
            {
                ioc_.run();
            });
        }
    }
    // consume tiles from the queue
    while (true)
    {
        tile_data tile;
        if (queue_.pop(tile))
        {
            if (++consumed_count_ == 16) return false;
            auto datasource_key = (boost::format("%1%-%2%-%3%-%4%") % datasource_hash_ % tile.zoom % tile.x % tile.y).str();
            auto itr = vector_tile_cache_.find(datasource_key);
            if (itr == vector_tile_cache_.end())
            {
                std::cerr << "\e[41m Consumed: #" << consumed_count_
                          << " " << tile.zoom << ":" << tile.x << ":" << tile.y << " num_tiles/consumed:"
                          << num_tiles_ << "/" << consumed_count_ << " available:" << queue_.read_available() << "\e[0m" << std::endl;
                vector_tile_cache_.emplace(datasource_key, tile.data);
            }
            else
            {
                std::cerr << "\e[42m Consumed: #" << consumed_count_
                          << " " << tile.zoom << ":" << tile.x << ":" << tile.y << " "
                          << tile.data.length() << " num_tiles/consumed:" << num_tiles_ << "/" << consumed_count_
                          << " available:" << queue_.read_available() << " STASHED\e[0m" << std::endl;
            }
            //
            std::string decompressed;
            mapnik::vector_tile_impl::zlib_decompress(tile.data.data(), tile.data.size(), decompressed);
            vector_tile_.reset(new mvt_io(std::move(decompressed), context_, tile.x, tile.y, zoom_, layer_));
            return true;
        }
        if (consumed_count_ == num_tiles_) break;
    }
    return false;
}
