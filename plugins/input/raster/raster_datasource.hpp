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

#ifndef RASTER_DATASOURCE_HPP
#define RASTER_DATASOURCE_HPP

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
#include <memory>

// stl
#include <vector>
#include <string>

DATASOURCE_PLUGIN_DEF(raster_datasource_plugin, raster);

class raster_datasource : public mapnik::datasource
{
  public:
    raster_datasource(const mapnik::parameters& params);
    virtual ~raster_datasource();
    datasource::datasource_t type() const;
    static const char* name();
    mapnik::featureset_ptr features(const mapnik::query& q) const;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    boost::optional<mapnik::datasource_geometry_t> get_geometry_type() const;
    mapnik::layer_descriptor get_descriptor() const;
    bool log_enabled() const;

  private:
    mapnik::layer_descriptor desc_;
    std::string filename_;
    std::string format_;
    mapnik::box2d<double> extent_;
    bool extent_initialized_;
    bool multi_tiles_;
    unsigned tile_size_;
    unsigned tile_stride_;
    unsigned width_;
    unsigned height_;
};

#endif // RASTER_DATASOURCE_HPP
