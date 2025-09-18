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
#include "raster_tiles_featureset.hpp"
#include "pmtiles_source.hpp"
#include "mbtiles_source.hpp"

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

raster_tiles_featureset::raster_tiles_featureset(std::string const& url_template,
                                                 mapnik::context_ptr const& ctx,
                                                 mapnik::box2d<double> const& extent,
                                                 int zoom,
                                                 int xmin,
                                                 int xmax,
                                                 int ymin,
                                                 int ymax,
                                                 std::unordered_map<std::string, std::string>& tiles_cache,
                                                 std::size_t datasource_hash,
                                                 double filter_factor)
    : url_template_(url_template),
      context_(ctx),
      extent_(extent),
      zoom_(zoom),
      xmin_(xmin),
      xmax_(xmax),
      ymin_(ymin),
      ymax_(ymax),
      tiles_cache_(tiles_cache),
      QUEUE_SIZE_((xmax - xmin + 1) * (ymax - ymin + 1)),
      queue_(QUEUE_SIZE_),
      stash_(ioc_, targets_, queue_),
      datasource_hash_(datasource_hash),
      filter_factor_(filter_factor)
{
    try
    {
        boost::urls::url url = boost::urls::format(url_template_, {{"z", zoom_}, {"x", 0}, {"y", 0}});
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

raster_tiles_featureset::~raster_tiles_featureset()
{
    for (std::size_t i = 0; i < workers_.size(); ++i)
    {
        workers_[i].join();
    }
}

mapnik::feature_ptr raster_tiles_featureset::next_feature(std::string const& image_buffer, int x, int y)
{
    std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(image_buffer.c_str(), image_buffer.size()));
    if (reader.get())
    {
        int image_width = reader->width();
        int image_height = reader->height();
        auto tile_bbox = tile_envelope(zoom_, x, y);
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

        mapnik::feature_ptr feature(mapnik::feature_factory::create(context_, std::hash<std::string>{}("FIXME")));
        mapnik::image_any data = reader->read(x_off, y_off, width, height);
        auto feature_raster_extent = t.backward(mapnik::box2d<double>(x_off, y_off, x_off + width, y_off + height));
        mapnik::raster_ptr raster =
          std::make_shared<mapnik::raster>(feature_raster_extent, intersect, std::move(data), filter_factor_);
        feature->set_raster(raster);
        return feature;
    }
    return mapnik::feature_ptr();
}

mapnik::feature_ptr raster_tiles_featureset::next()
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
                    std::unique_ptr<mapnik::tiles_source> source = nullptr;
                    if (url_template_.ends_with(".pmtiles"))
                    {
                        source = std::make_unique<mapnik::pmtiles_source>(url_template_);
                    }
                    else if (url_template_.ends_with(".mbtiles"))
                    {
                        source = std::make_unique<mapnik::mbtiles_source>(url_template_);
                    }
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
                    std::make_shared<worker_ssl>(ioc, ssl_ctx_, host_, port_, url_template_, stash_, std::ref(done_))
                      ->run();
                    ioc.run();
                });
            }
#endif
            else
            {
                workers_.emplace_back([this, reporting_work] {
                    boost::asio::io_context ioc;
                    std::make_shared<worker>(ioc, host_, port_, url_template_, stash_, std::ref(done_))->run();
                    ioc.run();
                });
            }
        }
        workers_.emplace_back([this] { ioc_.run(); });
    }
    // consume tiles from the queue
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
                return next_feature(buffer, tile.x, tile.y);
            }
            else if (tile.data)
            {
                if ((*tile.data).empty())
                {
                    continue;
                }
                tiles_cache_.emplace(datasource_key, *tile.data);
                return next_feature(*tile.data, tile.x, tile.y);
            }
        }
        if (consumed_count_ == num_tiles_)
            done_.store(true);
    }
    return mapnik::feature_ptr();
}
