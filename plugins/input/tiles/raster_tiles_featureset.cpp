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
#include <mapnik/image_reader.hpp>
#include <mapnik/raster.hpp>
#include <mapnik/image.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/feature_factory.hpp>
#include "tiles_source.hpp"
#include "raster_tiles_featureset.hpp"

// boost
#include <boost/format.hpp>

namespace {

inline mapnik::box2d<double> tile_envelope(int z, int x, int y)
{
    int tile_count = 1 << z;
    double x0 = x * (mapnik::EARTH_CIRCUMFERENCE / tile_count) - 0.5 * mapnik::EARTH_CIRCUMFERENCE;
    double y0 = -y * (mapnik::EARTH_CIRCUMFERENCE / tile_count) + 0.5 * mapnik::EARTH_CIRCUMFERENCE;
    double x1 = (x + 1) * (mapnik::EARTH_CIRCUMFERENCE / tile_count) - 0.5 * mapnik::EARTH_CIRCUMFERENCE;
    double y1 = -(y + 1) * (mapnik::EARTH_CIRCUMFERENCE / tile_count) + 0.5 * mapnik::EARTH_CIRCUMFERENCE;
    return mapnik::box2d<double>(x0, y0, x1, y1);
}

} // namespace

raster_tiles_featureset::raster_tiles_featureset(std::shared_ptr<mapnik::tiles_source> source_ptr,
                                                 mapnik::context_ptr const& ctx,
                                                 int zoom,
                                                 mapnik::box2d<double> const& extent,
                                                 std::string const& layer,
                                                 std::unordered_map<std::string, std::string>& vector_tile_cache,
                                                 std::size_t datasource_hash,
                                                 double filter_factor)
    : source_ptr_(source_ptr),
      context_(ctx),
      zoom_(zoom),
      extent_(extent),
      layer_(layer),
      vector_tile_cache_(vector_tile_cache),
      datasource_hash_(datasource_hash),
      filter_factor_(filter_factor)
{
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
    status_ = true;
}

raster_tiles_featureset::~raster_tiles_featureset() {}

mapnik::feature_ptr raster_tiles_featureset::next_feature()
{
    auto tile_key = (boost::format("%1%-%2%-%3%-%4%") % datasource_hash_ % zoom_ % x_ % y_).str();
    auto itr = vector_tile_cache_.find(tile_key);
    if (itr == vector_tile_cache_.end())
    {
        auto image_buffer = source_ptr_->get_tile(zoom_, x_, y_);
        auto result = vector_tile_cache_.emplace(tile_key, image_buffer);
        if (result.second)
            itr = result.first;
    }

    if (itr != vector_tile_cache_.end())
    {
        std::string const& image_buffer = itr->second;
        if (image_buffer.size() == 0)
            return mapnik::feature_ptr();
        std::unique_ptr<mapnik::image_reader> reader(
          mapnik::get_image_reader(image_buffer.c_str(), image_buffer.size()));
        if (reader.get())
        {
            int image_width = reader->width();
            int image_height = reader->height();
            auto tile_bbox = tile_envelope(zoom_, x_, y_);
            mapnik::view_transform t(image_width, image_height, tile_bbox, 0, 0);
            mapnik::box2d<double> intersect = extent_.intersect(tile_bbox);
            mapnik::box2d<double> ext = t.forward(intersect);
            // select minimum raster containing whole ext
            int x_off = static_cast<int>(std::floor(ext.minx()));
            int y_off = static_cast<int>(std::floor(ext.miny()));
            int end_x = static_cast<int>(std::ceil(ext.maxx()));
            int end_y = static_cast<int>(std::ceil(ext.maxy()));
            // clip to available data
            if (x_off >= image_width)
                x_off = image_width - 1;
            if (y_off >= image_height)
                y_off = image_height - 1;
            if (x_off < 0)
                x_off = 0;
            if (y_off < 0)
                y_off = 0;
            if (end_x > image_width)
                end_x = image_width;
            if (end_y > image_height)
                end_y = image_height;

            int width = end_x - x_off;
            int height = end_y - y_off;
            if (width < 1)
                width = 1;
            if (height < 1)
                height = 1;

            mapnik::feature_ptr feature(mapnik::feature_factory::create(context_, std::hash<std::string>{}(tile_key)));
            mapnik::image_any data = reader->read(x_off, y_off, width, height);
            auto feature_raster_extent = t.backward(mapnik::box2d<double>(x_off, y_off, x_off + width, y_off + height));
            mapnik::raster_ptr raster =
              std::make_shared<mapnik::raster>(feature_raster_extent, intersect, std::move(data), filter_factor_);
            feature->set_raster(raster);
            return feature;
        }
    }
    return mapnik::feature_ptr();
}

mapnik::feature_ptr raster_tiles_featureset::next()
{
    while (true)
    {
        if (!status_)
            return mapnik::feature_ptr();
        mapnik::feature_ptr feature = next_feature();
        status_ = next_tile();
        if (feature)
        {
            return feature;
        }
    }
    return mapnik::feature_ptr();
}

bool raster_tiles_featureset::next_tile()
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

bool raster_tiles_featureset::open_tile()
{
    return true;
}
