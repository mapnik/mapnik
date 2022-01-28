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

#ifndef OGR_DATASOURCE_HPP
#define OGR_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>

// boost
#include <boost/optional.hpp>
#include <memory>

// stl
#include <vector>
#include <string>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <ogrsf_frmts.h>
MAPNIK_DISABLE_WARNING_POP
#include "ogr_layer_ptr.hpp"

class ogr_datasource : public mapnik::datasource
{
  public:
    ogr_datasource(mapnik::parameters const& params);
    virtual ~ogr_datasource();
    mapnik::datasource::datasource_t type() const;
    static const char* name();
    mapnik::featureset_ptr features(mapnik::query const& q) const;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    boost::optional<mapnik::datasource_geometry_t> get_geometry_type() const;
    mapnik::layer_descriptor get_descriptor() const;

  private:
    void init(mapnik::parameters const& params);
    mapnik::box2d<double> extent_;
    mapnik::datasource::datasource_t type_;
    std::string dataset_name_;
    std::string index_name_;
    gdal_dataset_type dataset_;
    ogr_layer_ptr layer_;
    std::string layer_name_;
    mapnik::layer_descriptor desc_;
    bool indexed_;
};

#endif // OGR_DATASOURCE_HPP
