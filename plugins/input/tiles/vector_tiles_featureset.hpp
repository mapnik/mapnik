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

#ifndef VECTOR_TILES_FEATURESET_HPP
#define VECTOR_TILES_FEATURESET_HPP

#include <memory>
// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/datasource.hpp>
#include "xyz_tiles.hpp"
#include "mvt_io.hpp"

class vector_tiles_featureset : public mapnik::Featureset
{
  public:
    vector_tiles_featureset(std::string const& tiles_location,
                            mapnik::context_ptr const& ctx,
                            int zoom,
                            int xmin,
                            int xmax,
                            int ymin,
                            int ymax,
                            mapnik::box2d<double> const& extent,
                            std::string const& layer,
                            std::unordered_map<std::string, std::string>& tiles_cache,
                            std::size_t max_threads,
                            std::size_t datasource_hash);

    virtual ~vector_tiles_featureset();
    mapnik::feature_ptr next();

  private:
    mapnik::feature_ptr next_feature();
    bool valid() const;
    std::string tiles_location_;
    mapnik::context_ptr context_;
    boost::asio::io_context ioc_;
#if defined(MAPNIK_HAS_OPENSSL)
    boost::asio::ssl::context ssl_ctx_{boost::asio::ssl::context::tlsv12_client};
#endif
    int zoom_;
    int xmin_;
    int xmax_;
    int ymin_;
    int ymax_;
    mapnik::box2d<double> extent_;
    std::string const layer_;
    std::unique_ptr<mvt_io> vector_tile_;
    std::unordered_map<std::string, std::string>& tiles_cache_;
    std::size_t const QUEUE_SIZE_;
    queue_type queue_;
    std::vector<std::thread> workers_;
    std::vector<zxy> targets_;
    std::string host_;
    std::string port_;
    tiles_stash stash_;
    std::size_t num_tiles_{0};
    std::size_t consumed_count_{0};
    std::size_t max_threads_;
    std::size_t datasource_hash_;
    bool next_tile();
    bool first_ = true;
    bool ssl_ = false;
    bool local_file_ = true;
    std::atomic<bool> done_ = false;
};

#endif // VECTOR_TILES_FEATURESET_HPP
