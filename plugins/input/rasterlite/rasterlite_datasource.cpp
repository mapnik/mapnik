/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#include "rasterlite_datasource.hpp"
#include "rasterlite_featureset.hpp"

// boost

// mapnik
#include <mapnik/util/fs.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/boolean.hpp>
#include <mapnik/geom_util.hpp>

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(rasterlite_datasource)

using mapnik::box2d;
using mapnik::coord2d;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::datasource_exception;


/*
 * Opens a GDALDataset and returns a pointer to it.
 * Caller is responsible for calling GDALClose on it
 */
inline void* rasterlite_datasource::open_dataset() const
{
    void* dataset = rasterliteOpen (dataset_name_.c_str(), table_name_.c_str());

    if (! dataset)
    {
        throw datasource_exception("Rasterlite Plugin: Error opening dataset");
    }

    if (rasterliteIsError (dataset))
    {
        std::string error (rasterliteGetLastError(dataset));

        rasterliteClose (dataset);

        throw datasource_exception(error);
    }

    return dataset;
}


rasterlite_datasource::rasterlite_datasource(parameters const& params)
    : datasource(params),
      desc_(rasterlite_datasource::name(),"utf-8")
{
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_datasource: Initializing...";

    boost::optional<std::string> file = params.get<std::string>("file");
    if (!file) throw datasource_exception("missing <file> parameter");

    boost::optional<std::string> table = params.get<std::string>("table");
    if (!table) throw datasource_exception("missing <table> parameter");

    table_name_ = *table;

    boost::optional<std::string> base = params.get<std::string>("base");
    if (base)
        dataset_name_ = *base + "/" + *file;
    else
        dataset_name_ = *file;

    if (!mapnik::util::exists(dataset_name_)) throw datasource_exception(dataset_name_ + " does not exist");

    void *dataset = open_dataset();

    double x0, y0, x1, y1;
    if (rasterliteGetExtent (dataset, &x0, &y0, &x1, &y1) != RASTERLITE_OK)
    {
        std::string error (rasterliteGetLastError(dataset));

        rasterliteClose (dataset);

        throw datasource_exception(error);
    }

    extent_.init(x0,y0,x1,y1);

#ifdef MAPNIK_LOG
    int srid, auth_srid;
    const char *auth_name;
    const char *ref_sys_name;
    const char *proj4text;

    int tile_count;
    double pixel_x_size, pixel_y_size;
    int levels = rasterliteGetLevels (dataset);

    if (rasterliteGetSrid(dataset, &srid, &auth_name, &auth_srid, &ref_sys_name, &proj4text) != RASTERLITE_OK)
    {
        std::string error (rasterliteGetLastError(dataset));

        rasterliteClose (dataset);

        throw datasource_exception(error);
    }

    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_datasource: Data Source=" << rasterliteGetTablePrefix(dataset);
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_datasource: SRID=" << srid;
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_datasource: Authority=" << auth_name;
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_datasource: AuthSRID=" << auth_srid;
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_datasource: RefSys Name=" << ref_sys_name;
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_datasource: Proj4Text=" << proj4text;
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_datasource: Extent=" << x0 << "," << y0 << " " << x1 << "," << y1 << ")";
    MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_datasource: Levels=" << levels;

    for (int i = 0; i < levels; i++)
    {
        if (rasterliteGetResolution(dataset, i, &pixel_x_size, &pixel_y_size, &tile_count) == RASTERLITE_OK)
        {
            MAPNIK_LOG_DEBUG(rasterlite) << "rasterlite_datasource: Level=" << i
                                         << " x=" << pixel_x_size
                                         << " y=" << pixel_y_size
                                         << " tiles=" << tile_count;
        }
    }
#endif

    rasterliteClose(dataset);
}

rasterlite_datasource::~rasterlite_datasource()
{
}

const char * rasterlite_datasource::name()
{
    return "rasterlite";
}

mapnik::datasource::datasource_t rasterlite_datasource::type() const
{
    return datasource::Raster;
}

box2d<double> rasterlite_datasource::envelope() const
{
    return extent_;
}

boost::optional<mapnik::datasource_geometry_t> rasterlite_datasource::get_geometry_type() const
{
    return boost::optional<mapnik::datasource_geometry_t>();
}

layer_descriptor rasterlite_datasource::get_descriptor() const
{
    return desc_;
}

featureset_ptr rasterlite_datasource::features(query const& q) const
{
    rasterlite_query gq = q;
    return std::make_shared<rasterlite_featureset>(open_dataset(), gq);
}

featureset_ptr rasterlite_datasource::features_at_point(coord2d const& pt, double tol) const
{
    rasterlite_query gq = pt;
    return std::make_shared<rasterlite_featureset>(open_dataset(), gq);
}
