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

#ifndef GDAL_DATASOURCE_HPP
#define GDAL_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/datasource_plugin.hpp>
// boost
#include <boost/optional.hpp>

// stl
#include <vector>
#include <string>

// gdal
#include <gdal_priv.h>

DATASOURCE_PLUGIN_DEF(gdal_datasource_plugin, gdal);

class gdal_datasource : public mapnik::datasource
{
  public:
    gdal_datasource(mapnik::parameters const& params);
    virtual ~gdal_datasource();
    mapnik::datasource::datasource_t type() const override;
    static const char* name();
    mapnik::featureset_ptr features(mapnik::query const& q) const override;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const override;
    mapnik::box2d<double> envelope() const override;
    std::optional<mapnik::datasource_geometry_t> get_geometry_type() const override;
    mapnik::layer_descriptor get_descriptor() const override;

  private:
    std::unique_ptr<GDALDataset, decltype(&GDALClose)> dataset_;
    mapnik::box2d<double> extent_;
    std::string dataset_name_;
    int band_;
    mapnik::layer_descriptor desc_;
    unsigned width_;
    unsigned height_;
    double dx_;
    double dy_;
    int nbands_;
    bool shared_dataset_;
    std::optional<double> nodata_value_;
    double nodata_tolerance_;
    int64_t max_image_area_;
};

#endif // GDAL_DATASOURCE_HPP
