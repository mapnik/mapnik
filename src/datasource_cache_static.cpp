/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

// boost
#include <boost/assign/list_of.hpp>
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
#endif

namespace mapnik {

#ifdef MAPNIK_STATIC_PLUGINS
template<typename T>
datasource_ptr ds_generator(parameters const& params)
{
    return std::make_shared<T>(params);
}

typedef datasource_ptr (*ds_generator_ptr)(parameters const& params);
using datasource_map = std::unordered_map<std::string, ds_generator_ptr>;

static datasource_map ds_map = boost::assign::map_list_of
    #if defined(MAPNIK_STATIC_PLUGIN_CSV)
        (std::string("csv"), &ds_generator<csv_datasource>)
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_GDAL)
        (std::string("gdal"), &ds_generator<gdal_datasource>)
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_GEOJSON)
        (std::string("geojson"), &ds_generator<geojson_datasource>)
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_OCCI)
        (std::string("occi"), &ds_generator<occi_datasource>)
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_OGR)
        (std::string("ogr"), &ds_generator<ogr_datasource>)
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_OSM)
        (std::string("osm"), &ds_generator<osm_datasource>)
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_POSTGIS)
        (std::string("postgis"), &ds_generator<postgis_datasource>)
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_RASTER)
        (std::string("raster"), &ds_generator<raster_datasource>)
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_RASTERLITE)
        (std::string("rasterlite"), &ds_generator<rasterlite_datasource>)
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_SHAPE)
        (std::string("shape"), &ds_generator<shape_datasource>)
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_SQLITE)
        (std::string("sqlite"), &ds_generator<sqlite_datasource>)
    #endif
;
#endif

#ifdef MAPNIK_STATIC_PLUGINS
datasource_ptr create_static_datasource(parameters const& params)
{
    datasource_ptr ds;
    boost::optional<std::string> type = params.get<std::string>("type");
    datasource_map::iterator it = ds_map.find(*type);
    if (it != ds_map.end())
    {
        ds = it->second(params);
    }
    return ds;
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

}
