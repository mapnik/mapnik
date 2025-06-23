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
#include "mvt_io.hpp"

namespace mapnik {
class tiles_source; // fwd decl
}

class vector_tiles_featureset : public mapnik::Featureset
{
  public:
    vector_tiles_featureset(std::shared_ptr<mapnik::tiles_source> source_ptr,
                     mapnik::context_ptr const& ctx,
                     int const zoom,
                     mapnik::box2d<double> const& extent,
                     std::string const& layer,
                     std::unordered_map<std::string, std::string>& vector_tile_cache,
                     std::size_t datasource_hash);

    virtual ~vector_tiles_featureset();
    mapnik::feature_ptr next();

  private:
    mapnik::feature_ptr next_feature();
    bool valid() const;
    std::shared_ptr<mapnik::tiles_source> source_ptr_;
    mapnik::context_ptr context_;
    int zoom_;
    mapnik::box2d<double> extent_;
    std::string const layer_;
    std::unique_ptr<mvt_io> vector_tile_;
    std::unordered_map<std::string, std::string>& vector_tile_cache_;
    int xmin_;
    int xmax_;
    int ymin_;
    int ymax_;
    /// x index of the currently accessed tile
    int x_ = 0;
    /// y index of the currently accessed tile
    int y_ = 0;
    std::size_t datasource_hash_;
    bool next_tile();
    bool open_tile();
};

#endif // VECTOR_TILES_FEATURESET_HPP
