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

#ifndef GDAL_FEATURESET_HPP
#define GDAL_FEATURESET_HPP

// mapnik
#include <mapnik/featureset.hpp>
#include <mapnik/query.hpp>
#include <mapnik/util/variant.hpp>
// boost
#include <boost/optional.hpp>

class GDALDataset;
class GDALRasterBand;

using gdal_query = mapnik::util::variant<mapnik::query, mapnik::coord2d>;

class gdal_featureset : public mapnik::Featureset
{
    struct query_dispatch
    {
        query_dispatch( gdal_featureset & featureset)
            : featureset_(featureset) {}

        mapnik::feature_ptr operator() (mapnik::query const& q) const
        {
            return featureset_.get_feature(q);
        }

        mapnik::feature_ptr operator() (mapnik::coord2d const& p) const
        {
            return featureset_.get_feature_at_point(p);
        }

        gdal_featureset & featureset_;
    };

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
                    boost::optional<double> const& nodata,
                    double nodata_tolerance,
                    int64_t max_image_area);
    virtual ~gdal_featureset();
    mapnik::feature_ptr next();

private:
    void find_best_overview(int bandNumber,
                            int ideal_width,
                            int ideal_height,
                            int & current_width,
                            int & current_height) const;
    
    void find_best_overview(GDALRasterBand * band,
                            int ideal_width,
                            int ideal_height,
                            int & current_width,
                            int & current_height) const;

    mapnik::feature_ptr get_feature(mapnik::query const& q);
    mapnik::feature_ptr get_feature_at_point(mapnik::coord2d const& p);
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
    boost::optional<double> nodata_value_;
    double nodata_tolerance_;
    int64_t max_image_area_;
    bool first_;
};

#endif // GDAL_FEATURESET_HPP
