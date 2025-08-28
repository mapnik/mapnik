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

#include "tiles_datasource.hpp"
#include "vector_tiles_featureset.hpp"
#include "raster_tiles_featureset.hpp"
#include "xyz_featureset.hpp"
#include "pmtiles_source.hpp"
#include "mbtiles_source.hpp"
#include "vector_tile_projection.hpp"
#include <mapnik/geom_util.hpp>
#include <mapnik/util/fs.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/datasource_plugin.hpp>
#include <mapnik/well_known_srs.hpp>
#include <string>
#include <algorithm>
#include <thread>

DATASOURCE_PLUGIN_IMPL(tiles_datasource_plugin, tiles_datasource);
DATASOURCE_PLUGIN_EXPORT(tiles_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_AFTER_LOAD(tiles_datasource_plugin);
DATASOURCE_PLUGIN_EMPTY_BEFORE_UNLOAD(tiles_datasource_plugin);

tiles_datasource::tiles_datasource(mapnik::parameters const& params)
    : datasource(params),
      desc_(tiles_datasource::name(), *params.get<std::string>("encoding", "utf-8"))
{
    init(params);
}

tiles_datasource::~tiles_datasource() {}

mapnik::datasource::datasource_t tiles_datasource::type() const
{
    if (source_ptr_ && source_ptr_->is_raster())
        return datasource::Raster;
    return datasource::Vector;
}

char const* tiles_datasource::name()
{
    return "tiles";
}

mapnik::layer_descriptor tiles_datasource::get_descriptor() const
{
    return desc_;
}

std::optional<mapnik::datasource_geometry_t> tiles_datasource::get_geometry_type() const
{
    return mapnik::datasource_geometry_t::Collection;
}

mapnik::box2d<double> tiles_datasource::envelope() const
{
    return extent_;
}

namespace {

boost::json::value const& xyz_metadata(std::string const& url_str)
{
    static thread_local std::unordered_map<std::string, boost::json::value> metadata_cache;
    auto itr = metadata_cache.find(url_str);
    if (itr == metadata_cache.end())
    {
        auto meta = xyz_tiles::metadata(url_str);
        auto result = metadata_cache.emplace(url_str, std::move(meta));
        return result.first->second;
    }
    return itr->second;
}
} //

void tiles_datasource::init_metadata(boost::json::value const& metadata)
{
    auto layers = metadata.at("vector_layers");
    bool found = false;
    for (auto const& layer : layers.as_array())
    {
        std::string id = layer.at("id").as_string().c_str();
        if (id == layer_)
        {
            found = true;
            if (auto const* p = layer.as_object().if_contains("minzoom"))
            {
                minzoom_ = p->as_int64();//std::max(minzoom_, p->as_int64());
            }
            if (auto const* p = layer.as_object().if_contains("maxzoom"))
            {
                maxzoom_ = p->as_int64();//std::min(maxzoom_, p->as_int64());
            }
            for (auto const& field : layer.at("fields").as_object())
            {
                std::string name{field.key_c_str()};
                if (field.value() == "String")
                {
                    desc_.add_descriptor(mapnik::attribute_descriptor(name, mapnik::String));
                }
                else if (field.value() == "Number")
                {
                    desc_.add_descriptor(mapnik::attribute_descriptor(name, mapnik::Float));
                }
                else if (field.value() == "Boolean")
                {
                    desc_.add_descriptor(mapnik::attribute_descriptor(name, mapnik::Boolean));
                }
            }
            break;
        }
    }
    if (!found)
    {
        throw mapnik::datasource_exception("Requested layer '" + layer_ + "' not found.");
    }
}

void tiles_datasource::init(mapnik::parameters const& params)
{
    std::optional<std::string> file = params.get<std::string>("file");
    std::optional<std::string> json_url = params.get<std::string>("url");
    if (!file && !json_url)
    {
        throw mapnik::datasource_exception("Tiles Plugin: missing <file> or <json-url> parameter");
    }

    if (file)
    {
        std::optional<std::string> base = params.get<std::string>("base");
        if (base)
        {
            database_path_ = *base + "/" + *file;
        }
        else
        {
            database_path_ = *file;
        }
        if (!mapnik::util::exists(database_path_))
        {
            throw mapnik::datasource_exception("Tiles Plugin: " + database_path_ + " does not exist");
        }
        if (database_path_.ends_with(".pmtiles"))
        {
            source_ptr_ = std::make_shared<mapnik::pmtiles_source>(database_path_);
        }
        else if (database_path_.ends_with(".mbtiles"))
        {
            source_ptr_ = std::make_shared<mapnik::mbtiles_source>(database_path_);
        }
        else
        {
            throw mapnik::datasource_exception("Unexpected file extension in tiles source: " + database_path_);
        }

        if (!source_ptr_->is_raster())
        {
            std::optional<std::string> layer = params.get<std::string>("layer");
            try
            {
                layer_ = layer.value();
            }
            catch (std::bad_optional_access&)
            {
                throw mapnik::datasource_exception("Tiles Plugin: parameter 'layer' is missing.");
            }
        }

        minzoom_ = source_ptr_->minzoom();
        maxzoom_ = source_ptr_->maxzoom();
        extent_ = source_ptr_->extent();
        // overwrite envelope with user supplied
        std::optional<std::string> ext = params.get<std::string>("extent");
        if (ext && !ext->empty())
        {
            extent_.from_string(*ext);
        }
        if (!extent_.valid())
        {
            throw mapnik::datasource_exception("Tiles Plugin: " + database_path_ + " extent is invalid.");
        }
        // Bounds are specified in EPSG:4326, therefore transformation is required.
        mapnik::lonlat2merc(extent_.minx_, extent_.miny_);
        mapnik::lonlat2merc(extent_.maxx_, extent_.maxy_);
        if (!source_ptr_->is_raster())
        {
            auto metadata = source_ptr_->metadata();
            init_metadata(metadata);
        }
    }
    else if (json_url)
    {
        std::optional<std::string> layer = params.get<std::string>("layer");
        try
        {
            layer_ = layer.value();
        }
        catch (std::bad_optional_access&)
        {
            throw mapnik::datasource_exception("Tiles Plugin: parameter 'layer' is missing.");
        }

        extent_ = mapnik::box2d<double>(-180, -85.0511287, 180, 85.0511287);
        // overwrite envelope with user supplied
        std::optional<std::string> ext = params.get<std::string>("extent");
        if (ext && !ext->empty())
        {
            extent_.from_string(*ext);
        }
        if (!extent_.valid())
        {
            throw mapnik::datasource_exception("Tiles Plugin: " + database_path_ + " extent is invalid.");
        }
        // Bounds are specified in EPSG:4326, therefore transformation is required.
        mapnik::lonlat2merc(extent_.minx_, extent_.miny_);
        mapnik::lonlat2merc(extent_.maxx_, extent_.maxy_);
        auto metadata = xyz_metadata(*json_url);
        init_metadata(metadata);
        std::cerr << "min/max zoom: " << minzoom_ << "," << maxzoom_ << std::endl;
        auto tiles = metadata.at("tiles");
        for (auto const& tiles_url : tiles.as_array())
        {
            url_template_ = tiles_url.as_string();
        }
    }
}

mapnik::context_ptr tiles_datasource::get_context_with_attributes() const
{
    mapnik::context_ptr context = std::make_shared<mapnik::context_type>();
    std::vector<mapnik::attribute_descriptor> const& desc_ar = desc_.get_descriptors();
    for (auto const& attr_info : desc_ar)
    {
        context->push(attr_info.get_name());
    }
    return context;
}

mapnik::context_ptr tiles_datasource::get_query_context(mapnik::query const& q) const
{
    mapnik::context_ptr context = std::make_shared<mapnik::context_type>();
    std::vector<mapnik::attribute_descriptor> const& desc_ar = desc_.get_descriptors();
    for (auto const& name : q.property_names())
    {
        for (auto const& attr_info : desc_ar)
        {
            if (name == attr_info.get_name())
            {
                context->push(name);
                break;
            }
        }
    }
    return context;
}

namespace {

constexpr double scales[] = {279541132.014, 139770566.007, 69885283.0036, 34942641.5018, 17471320.7509, 8735660.37545,
                             4367830.18772, 2183915.09386, 1091957.54693, 545978.773466, 272989.386733, 136494.693366,
                             68247.3466832, 34123.6733416, 17061.8366708, 8530.9183354,  4265.4591677,  2132.72958385,
                             1066.36479192, 533.182395962, 266.59119798,  133.29559899,  66.6477994952};

std::int64_t scale_to_zoom(double scale, std::int64_t minzoom, std::int64_t maxzoom)
{
    for (std::int64_t zoom = 0; zoom < 22; ++zoom)
    {
        if (scale > std::ceil(scales[zoom]))
            return std::min(zoom, minzoom);
        else if (scale < std::ceil(scales[zoom]) && scale > std::ceil(scales[zoom + 1]))
        {
            return std::min(zoom, maxzoom);
        }
    }
    return maxzoom;
}
} // namespace

std::unordered_map<std::string, std::string>& tiles_datasource::tile_cache()
{
    static thread_local std::unordered_map<std::string, std::string> vector_tile_cache;
    return vector_tile_cache;
}

mapnik::featureset_ptr tiles_datasource::features(mapnik::query const& q) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "tiles_datasource::features");
#endif
    auto& vector_tile_cache = tile_cache();
    if (vector_tile_cache.size() > 64)
    {
        vector_tile_cache.clear();
    }

    mapnik::box2d<double> bbox = q.get_bbox().intersect(extent_);
    auto zoom = scale_to_zoom(q.scale_denominator(), minzoom_, maxzoom_);
    mapnik::context_ptr context = get_query_context(q);

    if (url_template_)
    {
        bbox.set_minx(bbox.minx() + 1e-6);
        bbox.set_maxx(bbox.maxx() - 1e-6);
        bbox.set_miny(bbox.miny() + 1e-6);
        bbox.set_maxy(bbox.maxy() - 1e-6);

        int tile_count = 1 << zoom;
        auto xmin = static_cast<int>((bbox.minx() + mapnik::EARTH_CIRCUMFERENCE / 2) *
                                 (tile_count / mapnik::EARTH_CIRCUMFERENCE));
        auto xmax = static_cast<int>((bbox.maxx() + mapnik::EARTH_CIRCUMFERENCE / 2) *
                                 (tile_count / mapnik::EARTH_CIRCUMFERENCE));
        auto ymin = static_cast<int>(((mapnik::EARTH_CIRCUMFERENCE / 2) - bbox.maxy()) *
                                 (tile_count / mapnik::EARTH_CIRCUMFERENCE));
        auto ymax = static_cast<int>(((mapnik::EARTH_CIRCUMFERENCE / 2) - bbox.miny()) *
                                 (tile_count / mapnik::EARTH_CIRCUMFERENCE));
        auto datasource_hash = std::hash<std::string>{}(*url_template_);
        return std::make_shared<xyz_featureset>(*url_template_,
                                                context,
                                                zoom,
                                                xmin, xmax, ymin, ymax,
                                                layer_,
                                                vector_tile_cache,
                                                datasource_hash);
    }

    if (source_ptr_)
    {
        auto datasource_hash = std::hash<std::string>{}(database_path_);
        if (source_ptr_->is_raster())
        {
            return std::make_shared<raster_tiles_featureset>(source_ptr_,
                                                             context,
                                                             zoom,
                                                             bbox,
                                                             layer_,
                                                             vector_tile_cache,
                                                             datasource_hash,
                                                             q.get_filter_factor());
        }
        else
        {
            return std::make_shared<vector_tiles_featureset>(source_ptr_,
                                                             context,
                                                             zoom,
                                                             bbox,
                                                             layer_,
                                                             vector_tile_cache,
                                                             datasource_hash);
        }
    }

    return mapnik::featureset_ptr{};
}

mapnik::featureset_ptr tiles_datasource::features_at_point(mapnik::coord2d const& pt, double tol) const
{
#ifdef MAPNIK_STATS
    mapnik::progress_timer __stats__(std::clog, "tiles_datasource::features_at_point");
#endif

    mapnik::filter_at_point filter(pt, tol);
    mapnik::context_ptr context = get_context_with_attributes();
    auto datasource_hash = std::hash<std::string>{}(database_path_);
    int tile_count = 1 << maxzoom_;
    auto tile_x =
      static_cast<int>((pt.x + mapnik::EARTH_CIRCUMFERENCE / 2) * (tile_count / mapnik::EARTH_CIRCUMFERENCE));
    auto tile_y =
      static_cast<int>(((mapnik::EARTH_CIRCUMFERENCE / 2) - pt.y) * (tile_count / mapnik::EARTH_CIRCUMFERENCE));

    double x0 = tile_x * (mapnik::EARTH_CIRCUMFERENCE / tile_count) - 0.5 * mapnik::EARTH_CIRCUMFERENCE;
    double y0 = -tile_y * (mapnik::EARTH_CIRCUMFERENCE / tile_count) + 0.5 * mapnik::EARTH_CIRCUMFERENCE;
    double x1 = (tile_x + 1) * (mapnik::EARTH_CIRCUMFERENCE / tile_count) - 0.5 * mapnik::EARTH_CIRCUMFERENCE;
    double y1 = -(tile_y + 1) * (mapnik::EARTH_CIRCUMFERENCE / tile_count) + 0.5 * mapnik::EARTH_CIRCUMFERENCE;
    auto query_bbox = mapnik::box2d<double>{x0, y0, x1, y1};
    if (url_template_)
    {
        auto datasource_hash = std::hash<std::string>{}(*url_template_);
        return std::make_shared<xyz_featureset>(*url_template_,
                                                context,
                                                maxzoom_,
                                                tile_x,
                                                tile_x,
                                                tile_y,
                                                tile_y,
                                                layer_,
                                                tile_cache(),
                                                datasource_hash);
    }
    if (!source_ptr_->is_raster())
        return std::make_shared<vector_tiles_featureset>(source_ptr_,
                                                         context,
                                                         maxzoom_,
                                                         query_bbox,
                                                         layer_,
                                                         tile_cache(),
                                                         datasource_hash);
    return mapnik::featureset_ptr();
}

#if !defined(_MSC_VER)
// Boost.Json header only
#include <boost/json/src.hpp>
#endif
