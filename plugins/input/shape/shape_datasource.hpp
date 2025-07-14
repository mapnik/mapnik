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

#ifndef SHAPE_HPP
#define SHAPE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/value/types.hpp>
#include <mapnik/datasource_plugin.hpp>

// stl
#include <string>

#include "shape_io.hpp"

using mapnik::coord2d;
using mapnik::datasource;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::parameters;
using mapnik::query;

DATASOURCE_PLUGIN_DEF(shape_datasource_plugin, shape);

class shape_datasource : public datasource
{
  public:
    shape_datasource(parameters const& params);
    virtual ~shape_datasource();
    datasource::datasource_t type() const override;
    static char const* name();
    featureset_ptr features(query const& q) const override;
    featureset_ptr features_at_point(coord2d const& pt, double tol = 0) const override;
    box2d<double> envelope() const override;
    std::optional<mapnik::datasource_geometry_t> get_geometry_type() const override;
    layer_descriptor get_descriptor() const override;

  private:
    void init(shape_io& shape);

    datasource::datasource_t type_;
    std::string shape_name_;
    shape_io::shapeType shape_type_;
    long file_length_;
    box2d<double> extent_;
    bool indexed_;
    int const row_limit_;
    layer_descriptor desc_;
};

#endif // SHAPE_HPP
