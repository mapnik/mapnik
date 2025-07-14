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
#include <mapnik/datasource_cache.hpp>
#include <mapnik/datasource.hpp>

#ifdef MAPNIK_STATIC_PLUGINS
#include <mapnik/params.hpp>
#include <mapnik/datasource_plugin.hpp>
#endif

// stl
#include <stdexcept>
#include <unordered_map>

// static plugin linkage
#ifdef MAPNIK_STATIC_PLUGINS
#if defined(MAPNIK_STATIC_PLUGIN_CSV)
#include "input/csv/csv_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_GDAL)
#include "input/gdal/gdal_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_GEOBUF)
#include "input/geobuf/geobuf_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_GEOJSON)
#include "input/geojson/geojson_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_GEOS)
#include "input/geos/geos_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_KISMET)
#include "input/kismet/kismet_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_OCCI)
#include "input/occi/occi_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_OGR)
#include "input/ogr/ogr_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_PGRASTER)
#include "input/pgraster/pgraster_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_OSM)
#include "input/osm/osm_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_POSTGIS)
#include "input/postgis/postgis_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_RASTER)
#include "input/raster/raster_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_RASTERLITE)
#include "input/rasterlite/rasterlite_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_SHAPE)
#include "input/shape/shape_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_SQLITE)
#include "input/sqlite/sqlite_datasource.hpp"
#endif
#if defined(MAPNIK_STATIC_PLUGIN_TOPOJSON)
#include "input/topojson/topojson_datasource.hpp"
#endif
#endif

#define REGISTER_STATIC_DATASOURCE_PLUGIN(classname)                                                                   \
    {                                                                                                                  \
        auto plugin = std::make_shared<classname>();                                                                   \
        plugin->after_load();                                                                                          \
        ds_map.emplace(std::string{classname::kName}, std::move(plugin));                                              \
    }
namespace mapnik {

#ifdef MAPNIK_STATIC_PLUGINS

template<typename T>
datasource_ptr ds_generator(parameters const& params)
{
    return std::make_shared<T>(params);
}

typedef datasource_ptr (*ds_generator_ptr)(parameters const& params);
using datasource_map = std::unordered_map<std::string, std::shared_ptr<datasource_plugin>>;

static datasource_map ds_map{};

void init_datasource_cache_static()
{
#if defined(MAPNIK_STATIC_PLUGIN_CSV)
    REGISTER_STATIC_DATASOURCE_PLUGIN(csv_datasource_plugin);
#endif
#if defined(MAPNIK_STATIC_PLUGIN_GDAL)
    REGISTER_STATIC_DATASOURCE_PLUGIN(gdal_datasource_plugin);
#endif
#if defined(MAPNIK_STATIC_PLUGIN_GEOBUF)
    REGISTER_STATIC_DATASOURCE_PLUGIN(geobuf_datasource_plugin);
#endif
#if defined(MAPNIK_STATIC_PLUGIN_GEOJSON)
    REGISTER_STATIC_DATASOURCE_PLUGIN(geojson_datasource_plugin);
#endif
#if defined(MAPNIK_STATIC_PLUGIN_OCCI)
    REGISTER_STATIC_DATASOURCE_PLUGIN(occi_datasource_plugin);
#endif
#if defined(MAPNIK_STATIC_PLUGIN_OGR)
    REGISTER_STATIC_DATASOURCE_PLUGIN(ogr_datasource_plugin);
#endif
#if defined(MAPNIK_STATIC_PLUGIN_PGRASTER)
    REGISTER_STATIC_DATASOURCE_PLUGIN(pgraster_datasource_plugin);
#endif
#if defined(MAPNIK_STATIC_PLUGIN_OSM)
    REGISTER_STATIC_DATASOURCE_PLUGIN(osm_datasource_plugin);
#endif
#if defined(MAPNIK_STATIC_PLUGIN_POSTGIS)
    REGISTER_STATIC_DATASOURCE_PLUGIN(postgis_datasource_plugin);
#endif
#if defined(MAPNIK_STATIC_PLUGIN_RASTER)
    REGISTER_STATIC_DATASOURCE_PLUGIN(raster_datasource_plugin);
#endif
#if defined(MAPNIK_STATIC_PLUGIN_RASTERLITE)
    REGISTER_STATIC_DATASOURCE_PLUGIN(rasterlite_datasource_plugin);
#endif
#if defined(MAPNIK_STATIC_PLUGIN_SHAPE)
    REGISTER_STATIC_DATASOURCE_PLUGIN(shape_datasource_plugin);
#endif
#if defined(MAPNIK_STATIC_PLUGIN_SQLITE)
    REGISTER_STATIC_DATASOURCE_PLUGIN(sqlite_datasource_plugin);
#endif
#if defined(MAPNIK_STATIC_PLUGIN_TOPOJSON)
    REGISTER_STATIC_DATASOURCE_PLUGIN(topojson_datasource_plugin);
#endif
};

datasource_ptr create_static_datasource(parameters const& params)
{
    auto const type = params.get<std::string>("type");
    datasource_map::iterator it = ds_map.find(*type);
    if (it != ds_map.end())
    {
        return it->second->create(params);
    }
    return datasource_ptr{};
}
#else
datasource_ptr create_static_datasource(parameters const& /*params*/)
{
    return datasource_ptr();
}
#endif

std::vector<std::string> get_static_datasource_names()
{
    std::vector<std::string> names;

#ifdef MAPNIK_STATIC_PLUGINS
    datasource_map::iterator it = ds_map.begin();
    while (it != ds_map.end())
    {
        names.push_back(it->first);

        it++;
    }
#endif

    return names;
}

} // namespace mapnik
