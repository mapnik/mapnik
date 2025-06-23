/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2023 Geofabrik GmbH
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

#ifndef MAPNIK_MBTILES_SOURCE_HPP
#define MAPNIK_MBTILES_SOURCE_HPP

#include <mapnik/global.hpp>

#include "tiles_source.hpp"
#include "sqlite_connection.hpp"

// mapnik_vector_tile
#include "vector_tile_compression.hpp"
// boost
#include <boost/format.hpp>

namespace mapnik {

class mbtiles_source : public tiles_source
{
  private:
    std::uint8_t minzoom_ = 1;
    std::uint8_t maxzoom_ = 14;
    mapnik::box2d<double> extent_;
    std::string database_path_;
    std::shared_ptr<sqlite_connection> dataset_;
    std::string format_;
    inline std::uint8_t zoom_from_string(std::string const& str) { return std::stoi(str); }
    std::int32_t convert_y(std::int32_t y, std::uint8_t zoom) const { return (1 << zoom) - 1 - y; }

  public:
    mbtiles_source(std::string const& database_path)
        : database_path_(database_path)
    {
        int sqlite_mode = SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE;
        dataset_ = std::make_shared<sqlite_connection>(database_path_, sqlite_mode);
        auto result = dataset_->execute_query("SELECT value FROM metadata WHERE name = 'format';");
        if (!result->is_valid() || !result->step_next() || result->column_type(0) != SQLITE_TEXT)

        {
            throw mapnik::datasource_exception("MBTiles Plugin: Can't infer tile format value from " + database_path_);
        }
        format_ = result->column_text(0);

        if (format_ != "pbf" && format_ != "jpg" && format_ != "png" && format_ != "webp")
        {
            throw mapnik::datasource_exception("MBTiles Plugin: " + database_path_ + " has unsupported tile format '" +
                                               format_ + "', expected 'pbf|jpg|png|webp'.");
        }
        result = dataset_->execute_query("SELECT value FROM metadata WHERE name = 'bounds';");
        if (result->is_valid() && result->step_next() && result->column_type(0) == SQLITE_TEXT)
        {
            const std::string extent_str = result->column_text(0);
            if (extent_str.empty())
            {
                throw mapnik::datasource_exception("MBTiles Plugin: " + database_path_ + " has invalid extent.");
            }
            extent_.from_string(extent_str);
        }
        if (!extent_.valid())
        {
            throw mapnik::datasource_exception("MBTiles Plugin: " + database_path_ + " extent is invalid.");
        }
        result = dataset_->execute_query("SELECT value FROM metadata WHERE name = 'minzoom';");
        if (result->is_valid() && result->step_next() && result->column_type(0) == SQLITE_TEXT)
        {
            minzoom_ = zoom_from_string(result->column_text(0));
        }
        // initialize maxzoom
        result = dataset_->execute_query("SELECT value FROM metadata WHERE name = 'maxzoom';");
        if (result->is_valid() && result->step_next() && result->column_type(0) == SQLITE_TEXT)
        {
            maxzoom_ = zoom_from_string(result->column_text(0));
        }
    }

    std::uint8_t minzoom() const { return minzoom_; }

    std::uint8_t maxzoom() const { return maxzoom_; }

    mapnik::box2d<double> const& extent() const { return extent_; }

    std::string get_tile(std::uint8_t z, std::uint32_t x, std::uint32_t y) const
    {
        std::string sql =
          (boost::format(
             "SELECT tile_data FROM tiles WHERE zoom_level = %1% AND tile_column = %2% AND tile_row = %3%") %
           (int)z % x % convert_y(y, z))
            .str();
        std::shared_ptr<sqlite_resultset> result(dataset_->execute_query(sql));
        int size = 0;
        char const* blob = nullptr;
        if (result->is_valid() && result->step_next() && result->column_type(0) == SQLITE_BLOB)
        {
            blob = result->column_blob(0, size);
        }
        else
        {
            return std::string();
        }
        if (mapnik::vector_tile_impl::is_gzip_compressed(blob, size) ||
            mapnik::vector_tile_impl::is_zlib_compressed(blob, size))
        {
            std::string decompressed;
            mapnik::vector_tile_impl::zlib_decompress(blob, size, decompressed);
            return decompressed;
        }
        return std::string(blob, size);
    }

    boost::json::value metadata() const
    {
        std::string metadata;
        boost::json::value json_value;
        if (!is_raster())
        {
            auto result = dataset_->execute_query("SELECT value FROM metadata WHERE name = 'json';");
            if (result->is_valid() && result->step_next() && result->column_type(0) == SQLITE_TEXT)
            {
                metadata = result->column_text(0);
            }
            if (metadata.empty())
            {
                throw mapnik::datasource_exception("PMTiles plugin: " + database_path_ +
                                                   " has no 'json' entry in metadata table.");
            }
            try
            {
                json_value = boost::json::parse(metadata);
            }
            catch (std::exception const& ex)
            {
                std::cerr << ex.what() << std::endl;
            }
        }
        return json_value;
    }
    bool is_raster() const { return format_ == "png" || format_ == "jpg" || format_ == "webp"; }
};

} // namespace mapnik

#endif // MAPNIK_MBTILES_SOURCE_HPP
