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

#ifndef MAPNIK_TILE_SOURCE_HPP
#define MAPNIK_TILE_SOURCE_HPP

#include <boost/json.hpp>
#include <mapnik/geometry/envelope.hpp>

namespace mapnik {

class tiles_source
{
  public:
    virtual std::uint8_t minzoom() const = 0;
    virtual std::uint8_t maxzoom() const = 0;
    virtual mapnik::box2d<double> const& extent() const = 0;
    virtual boost::json::value metadata() const = 0;
    virtual std::string get_tile(std::uint8_t z, std::uint32_t x, std::uint32_t y) const = 0;
    virtual std::string
      get_tile_raw(std::uint8_t z, std::uint32_t x, std::uint32_t y) const = 0; // don't decompress, return raw data.
    virtual bool is_raster() const = 0;
    virtual ~tiles_source() = default;
    static std::unique_ptr<tiles_source> get_source(std::string const& filename);
};

} // namespace mapnik

#endif // MAPNIK_TILE_SOURCE_HPP
