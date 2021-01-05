/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

// boost

// mapnik
#include <mapnik/util/fs.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/boolean.hpp>

#include "raster_featureset.hpp"
#include "raster_info.hpp"
#include "raster_datasource.hpp"

using mapnik::layer_descriptor;
using mapnik::featureset_ptr;
using mapnik::query;
using mapnik::coord2d;
using mapnik::datasource_exception;
using mapnik::datasource;
using mapnik::parameters;
using mapnik::image_reader;

DATASOURCE_PLUGIN(raster_datasource)

raster_datasource::raster_datasource(parameters const& params)
  : datasource(params),
    desc_(raster_datasource::name(), "utf-8"),
    extent_initialized_(false)
{
    MAPNIK_LOG_DEBUG(raster) << "raster_datasource: Initializing...";

    boost::optional<std::string> file = params.get<std::string>("file");
    if (! file) throw datasource_exception("Raster Plugin: missing <file> parameter ");

    boost::optional<std::string> base = params.get<std::string>("base");
    if (base)
        filename_ = *base + "/" + *file;
    else
        filename_ = *file;

    multi_tiles_ = *params.get<mapnik::boolean_type>("multi", false);
    tile_size_ = *params.get<mapnik::value_integer>("tile_size", 256);
    tile_stride_ = *params.get<mapnik::value_integer>("tile_stride", 1);

    boost::optional<std::string> format_from_filename = mapnik::type_from_filename(*file);
    format_ = *params.get<std::string>("format",format_from_filename?(*format_from_filename) : "tiff");

    boost::optional<mapnik::value_double> lox = params.get<mapnik::value_double>("lox");
    boost::optional<mapnik::value_double> loy = params.get<mapnik::value_double>("loy");
    boost::optional<mapnik::value_double> hix = params.get<mapnik::value_double>("hix");
    boost::optional<mapnik::value_double> hiy = params.get<mapnik::value_double>("hiy");

    boost::optional<std::string> ext = params.get<std::string>("extent");

    if (lox && loy && hix && hiy)
    {
        extent_.init(*lox, *loy, *hix, *hiy);
        extent_initialized_ = true;
    }
    else if (ext)
    {
        extent_initialized_ = extent_.from_string(*ext);
    }
    else //bounding box from image_reader
    {
        std::unique_ptr<image_reader> reader(mapnik::get_image_reader(*file));
        if (!reader) throw datasource_exception("Raster Plugin: failed to create reader for " + *file);
        auto bbox = reader->bounding_box();
        if (bbox)
        {
            extent_ = *bbox;
            extent_initialized_ = true;
        }
    }

    if (! extent_initialized_)
    {
        throw datasource_exception("Raster Plugin: valid <extent> or <lox> <loy> <hix> <hiy> are required");
    }

    if (multi_tiles_)
    {
        boost::optional<mapnik::value_integer> x_width = params.get<mapnik::value_integer>("x_width");
        boost::optional<mapnik::value_integer> y_width = params.get<mapnik::value_integer>("y_width");

        if (! x_width)
        {
            throw datasource_exception("Raster Plugin: x-width parameter not supplied for multi-tiled data source.");
        }

        if (! y_width)
        {
            throw datasource_exception("Raster Plugin: y-width parameter not supplied for multi-tiled data source.");
        }

        width_ = x_width.get() * tile_size_;
        height_ = y_width.get() * tile_size_;
    }
    else
    {
        if (!mapnik::util::exists(filename_))
        {
            throw datasource_exception("Raster Plugin: " + filename_ + " does not exist");
        }

        try
        {
            std::unique_ptr<image_reader> reader(mapnik::get_image_reader(filename_, format_));
            if (reader.get())
            {
                width_ = reader->width();
                height_ = reader->height();
            }
        }
        catch (mapnik::image_reader_exception const& ex)
        {
            throw datasource_exception("Raster Plugin: image reader exception: " + std::string(ex.what()));
        }
        catch (std::exception const& ex)
        {
            throw datasource_exception("Raster Plugin: " + std::string(ex.what()));
        }
        catch (...)
        {
            throw datasource_exception("Raster Plugin: image reader unknown exception caught");
        }
    }

    MAPNIK_LOG_DEBUG(raster) << "raster_datasource: Raster size=" << width_ << "," << height_;

}

raster_datasource::~raster_datasource()
{
}

mapnik::datasource::datasource_t raster_datasource::type() const
{
    return datasource::Raster;
}

const char * raster_datasource::name()
{
    return "raster";
}

mapnik::box2d<double> raster_datasource::envelope() const
{
    return extent_;
}

boost::optional<mapnik::datasource_geometry_t> raster_datasource::get_geometry_type() const
{
    return boost::optional<mapnik::datasource_geometry_t>();
}

layer_descriptor raster_datasource::get_descriptor() const
{
    return desc_;
}

featureset_ptr raster_datasource::features(query const& q) const
{
    mapnik::view_transform t(width_, height_, extent_, 0, 0);
    mapnik::box2d<double> intersect = extent_.intersect(q.get_bbox());
    mapnik::box2d<double> ext = t.forward(intersect);

    const int width  = int(ext.maxx() + 0.5) - int(ext.minx() + 0.5);
    const int height = int(ext.maxy() + 0.5) - int(ext.miny() + 0.5);

    MAPNIK_LOG_DEBUG(raster) << "raster_datasource: Box size=" << width << "," << height;

    if (multi_tiles_)
    {
        MAPNIK_LOG_DEBUG(raster) << "raster_datasource: Multi-Tiled policy";

        tiled_multi_file_policy policy(filename_, format_, tile_size_, extent_, q.get_bbox(), width_, height_, tile_stride_);

        return std::make_shared<raster_featureset<tiled_multi_file_policy> >(policy, extent_, q);
    }
    else if (width * height > static_cast<int>(tile_size_ * tile_size_ << 2))
    {
        MAPNIK_LOG_DEBUG(raster) << "raster_datasource: Tiled policy";

        tiled_file_policy policy(filename_, format_, tile_size_, extent_, q.get_bbox(), width_, height_);

        return std::make_shared<raster_featureset<tiled_file_policy> >(policy, extent_, q);
    }
    else
    {
        MAPNIK_LOG_DEBUG(raster) << "raster_datasource: Single file";

        raster_info info(filename_, format_, extent_, width_, height_);
        single_file_policy policy(info);

        return std::make_shared<raster_featureset<single_file_policy> >(policy, extent_, q);
    }
}

featureset_ptr raster_datasource::features_at_point(coord2d const&, double tol) const
{
    MAPNIK_LOG_WARN(raster) << "raster_datasource: feature_at_point not supported";

    return mapnik::make_invalid_featureset();
}
