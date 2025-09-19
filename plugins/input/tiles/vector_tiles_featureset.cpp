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
#include "vector_tiles_featureset.hpp"
#include "tiles_source.hpp"
#include "vector_tile_compression.hpp"
// boost
#include <boost/format.hpp>

vector_tiles_featureset::vector_tiles_featureset(std::string const& tiles_location,
                                                 mapnik::context_ptr const& ctx,
                                                 int zoom,
                                                 int xmin,
                                                 int xmax,
                                                 int ymin,
                                                 int ymax,
                                                 std::string const& layer,
                                                 std::unordered_map<std::string, std::string>& tiles_cache,
                                                 std::size_t datasource_hash)
    : tiles_location_(tiles_location),
      context_(ctx),
      zoom_(zoom),
      xmin_(xmin),
      xmax_(xmax),
      ymin_(ymin),
      ymax_(ymax),
      layer_(layer),
      vector_tile_(nullptr),
      tiles_cache_(tiles_cache),
      QUEUE_SIZE_((xmax - xmin + 1) * (ymax - ymin + 1)),
      queue_(QUEUE_SIZE_),
      stash_(ioc_, targets_, queue_),
      datasource_hash_(datasource_hash)
{
    try
    {
        boost::urls::url url = boost::urls::format(tiles_location_, {{"z", zoom_}, {"x", 0}, {"y", 0}});
        host_ = url.host();
        auto scheme = url.scheme();
        if (scheme == "https")
        {
            ssl_ = true;
            local_file_ = false;
            port_ = url.port().empty() ? "443" : url.port();
        }
        else if (scheme == "http")
        {
            port_ = url.port().empty() ? "80" : url.port();
            local_file_ = false;
        }
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }
}

vector_tiles_featureset::~vector_tiles_featureset()
{
    for (std::size_t i = 0; i < workers_.size(); ++i)
    {
        workers_[i].join();
    }
}

bool vector_tiles_featureset::valid() const
{
    return vector_tile_.get() != nullptr;
}

mapnik::feature_ptr vector_tiles_featureset::next_feature()
{
    mapnik::feature_ptr f = mapnik::feature_ptr();
    if (valid())
    {
        f = vector_tile_->next();
    }
    return f;
}

mapnik::feature_ptr vector_tiles_featureset::next()
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

bool vector_tiles_featureset::next_tile()
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
                auto itr = tiles_cache_.find(datasource_key);
                if (itr == tiles_cache_.end())
                {
                    stash_.targets().emplace_back(zoom_, x, y);
                }
                else
                {
                    std::string buffer = itr->second;
                    stash_.push_async(tile_data(zoom_, x, y));
                }
            }
        }

        std::size_t max_threads = 4;
        std::size_t threads = std::min(max_threads, stash_.targets().size());
        workers_.reserve(threads + 1);
        for (std::size_t i = 0; i < threads; ++i)
        {
            auto reporting_work =
              boost::asio::require(ioc_.get_executor(), boost::asio::execution::outstanding_work.tracked);

            if (local_file_)
            {
                workers_.emplace_back([this, reporting_work] {
                    std::unique_ptr<mapnik::tiles_source> source = mapnik::tiles_source::get_source(tiles_location_);
                    if (source)
                    {
                        while (!done_)
                        {
                            auto zxy = stash_.get_zxy();
                            if (!zxy)
                                break;
                            auto data = source->get_tile_raw(std::get<0>(*zxy), std::get<1>(*zxy), std::get<2>(*zxy));
                            stash_.push_async(
                              tile_data{std::get<0>(*zxy), std::get<1>(*zxy), std::get<2>(*zxy), std::move(data)});
                        }
                    }
                });
            }
#if defined(MAPNIK_HAS_OPENSSL)
            else if (ssl_)
            {
                workers_.emplace_back([this, reporting_work] {
                    boost::asio::io_context ioc;
                    std::make_shared<worker_ssl>(ioc, ssl_ctx_, host_, port_, tiles_location_, stash_, std::ref(done_))
                      ->run();
                    ioc.run();
                });
            }
#endif
            else
            {
                workers_.emplace_back([this, reporting_work] {
                    boost::asio::io_context ioc;
                    std::make_shared<worker>(ioc, host_, port_, tiles_location_, stash_, std::ref(done_))->run();
                    ioc.run();
                });
            }
        }
        workers_.emplace_back([this] { ioc_.run(); });
    }
    // consume tiles from the queue
    bool status = false;
    while (!done_.load())
    {
        tile_data tile;
        if (queue_.pop(tile))
        {
            ++consumed_count_;
            auto datasource_key =
              (boost::format("%1%-%2%-%3%-%4%") % datasource_hash_ % tile.zoom % tile.x % tile.y).str();
            auto itr = tiles_cache_.find(datasource_key);
            if (itr != tiles_cache_.end())
            {
                auto buffer = itr->second;
                vector_tile_.reset(new mvt_io(std::move(buffer), context_, tile.x, tile.y, zoom_, layer_));
                status = true;
            }
            else if (tile.data)
            {
                if ((*tile.data).empty())
                    continue;
                std::string decompressed;
                mapnik::vector_tile_impl::zlib_decompress((*tile.data).data(), (*tile.data).size(), decompressed);
                tiles_cache_.emplace(datasource_key, decompressed);
                vector_tile_.reset(new mvt_io(std::move(decompressed), context_, tile.x, tile.y, zoom_, layer_));
                status = true;
            }
            if (consumed_count_ == QUEUE_SIZE_)
                done_.store(true);
            break;
        }
        if (consumed_count_ == num_tiles_)
            done_.store(true);
    }
    return status;
}
