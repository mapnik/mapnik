/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/debug.hpp>
#include <mapnik/datasource_cache.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/params.hpp>
#include <mapnik/plugin.hpp>

// boost
#include <boost/make_shared.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>

// stl
#include <algorithm>
#include <stdexcept>
#include <map>

// static plugin linkage
#ifdef MAPNIK_STATIC_PLUGINS
    #if defined(MAPNIK_STATIC_PLUGIN_CSV)
        #include "plugins/input/csv/csv_datasource.hpp"
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_GDAL)
        #include "plugins/input/gdal/gdal_datasource.hpp"
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_GEOJSON)
        #include "plugins/input/geojson/geojson_datasource.hpp"
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_GEOS)
        #include "plugins/input/geos/geos_datasource.hpp"
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_KISMET)
        #include "plugins/input/kismet/kismet_datasource.hpp"
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_OCCI)
        #include "plugins/input/occi/occi_datasource.hpp"
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_OGR)
        #include "plugins/input/ogr/ogr_datasource.hpp"
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_OSM)
        #include "plugins/input/osm/osm_datasource.hpp"
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_POSTGIS)
        #include "plugins/input/postgis/postgis_datasource.hpp"
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_PYTHON)
        #include "plugins/input/python/python_datasource.hpp"
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_RASTER)
        #include "plugins/input/raster/raster_datasource.hpp"
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_RASTERLITE)
        #include "plugins/input/rasterlite/rasterlite_datasource.hpp"
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_SHAPE)
        #include "plugins/input/shape/shape_datasource.hpp"
    #endif
    #if defined(MAPNIK_STATIC_PLUGIN_SQLITE)
        #include "plugins/input/sqlite/sqlite_datasource.hpp"
    #endif
#endif

namespace mapnik {

datasource_ptr create_static_datasource(parameters const& params)
{
    datasource_ptr ds;

#ifdef MAPNIK_STATIC_PLUGINS
    boost::optional<std::string> type = params.get<std::string>("type");

    if (*type == "csv")
    {
        #if defined(MAPNIK_STATIC_PLUGIN_CSV)
            ds = boost::make_shared<csv_datasource>(params);
        #endif
    }
    else if (*type == "gdal")
    {
        #if defined(MAPNIK_STATIC_PLUGIN_GDAL)
            ds = boost::make_shared<gdal_datasource>(params);
        #endif
    }
    else if (*type == "geojson")
    {
        #if defined(MAPNIK_STATIC_PLUGIN_GEOJSON)
            ds = boost::make_shared<geojson_datasource>(params);
        #endif
    }
    else if (*type == "geos")
    {
        #if defined(MAPNIK_STATIC_PLUGIN_GEOS)
            ds = boost::make_shared<geos_datasource>(params);
        #endif
    }
    else if (*type == "kismet")
    {
        #if defined(MAPNIK_STATIC_PLUGIN_KISMET)
            ds = boost::make_shared<kismet_datasource>(params);
        #endif
    }
    else if (*type == "occi")
    {
        #if defined(MAPNIK_STATIC_PLUGIN_OCCI)
            ds = boost::make_shared<occi_datasource>(params);
        #endif
    }
    else if (*type == "ogr")
    {
        #if defined(MAPNIK_STATIC_PLUGIN_OGR)
            ds = boost::make_shared<ogr_datasource>(params);
        #endif
    }
    else if (*type == "osm")
    {
        #if defined(MAPNIK_STATIC_PLUGIN_OSM)
            ds = boost::make_shared<osm_datasource>(params);
        #endif
    }
    else if (*type == "postgis")
    {
        #if defined(MAPNIK_STATIC_PLUGIN_POSTGIS)
            ds = boost::make_shared<postgis_datasource>(params);
        #endif
    }
    else if (*type == "python")
    {
        #if defined(MAPNIK_STATIC_PLUGIN_PYTHON)
            ds = boost::make_shared<python_datasource>(params);
        #endif
    }
    else if (*type == "raster")
    {
        #if defined(MAPNIK_STATIC_PLUGIN_RASTER)
            ds = boost::make_shared<raster_datasource>(params);
        #endif
    }
    else if (*type == "rasterlite")
    {
        #if defined(MAPNIK_STATIC_PLUGIN_RASTERLITE)
            ds = boost::make_shared<rasterlite_datasource>(params);
        #endif
    }
    else if (*type == "shape")
    {
        #if defined(MAPNIK_STATIC_PLUGIN_SHAPE)
            ds = boost::make_shared<shape_datasource>(params);
        #endif
    }
    else if (*type == "sqlite")
    {
        #if defined(MAPNIK_STATIC_PLUGIN_SQLITE)
            ds = boost::make_shared<sqlite_datasource>(params);
        #endif
    }

#endif

    return ds;
}

}
