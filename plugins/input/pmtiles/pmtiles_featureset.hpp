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


#ifndef PMTILES_FEATURESET_HPP_
#define PMTILES_FEATURESET_HPP_

#include <memory>
// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/datasource.hpp>
#include "mvt_io.hpp"


namespace mapnik {
class pmtiles_file; //fwd decl
}

class pmtiles_featureset : public mapnik::Featureset
{
public:
    pmtiles_featureset(std::shared_ptr<mapnik::pmtiles_file> file_ptr,
                       mapnik::context_ptr const& ctx,
                       int const zoom,
                       mapnik::box2d<double> const& extent,
                       std::string const& layer,
                       std::unordered_map<std::string,
                       std::string> & vector_tile_cache,
                       std::size_t datasource_hash);

    virtual ~pmtiles_featureset();
    mapnik::feature_ptr next();
private:
    mapnik::feature_ptr next_feature();
    bool valid() const;
    std::shared_ptr<mapnik::pmtiles_file> file_ptr_;
    mapnik::context_ptr context_;
    int zoom_;
    mapnik::box2d<double> const extent_;
    std::string const layer_;
    std::unique_ptr<mvt_io> vector_tile_;
    std::unordered_map<std::string, std::string> & vector_tile_cache_;
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

#endif /* PMTILES_FEATURESET_HPP_ */
