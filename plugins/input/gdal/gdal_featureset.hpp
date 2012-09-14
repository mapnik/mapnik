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

#ifndef GDAL_FEATURESET_HPP
#define GDAL_FEATURESET_HPP

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/datasource.hpp>

// boost
#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include "gdal_datasource.hpp"

class GDALDataset;
class GDALRasterBand;

typedef boost::variant<mapnik::query, mapnik::coord2d> gdal_query;

class gdal_featureset : public mapnik::Featureset
{
public:
    gdal_featureset(GDALDataset& dataset,
                    int band,
                    gdal_query q,
                    mapnik::box2d<double> extent,
                    unsigned width,
                    unsigned height,
                    int nbands,
                    double dx,
                    double dy,
                    double filter_factor,
                    boost::optional<double> const& nodata);
    virtual ~gdal_featureset();
    mapnik::feature_ptr next();

private:
    mapnik::feature_ptr get_feature(mapnik::query const& q);
    mapnik::feature_ptr get_feature_at_point(mapnik::coord2d const& p);

#ifdef MAPNIK_LOG
    void get_overview_meta(GDALRasterBand * band);
#endif

    GDALDataset & dataset_;
    mapnik::context_ptr ctx_;
    int band_;
    gdal_query gquery_;
    mapnik::box2d<double> raster_extent_;
    unsigned raster_width_;
    unsigned raster_height_;
    double dx_;
    double dy_;
    int nbands_;
    double filter_factor_;
    boost::optional<double> nodata_value_;
    bool first_;
};

#endif // GDAL_FEATURESET_HPP
