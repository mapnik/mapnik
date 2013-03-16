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

#ifndef MAPNIK_MEMORY_DATASOURCE_HPP
#define MAPNIK_MEMORY_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/feature_layer_desc.hpp>

// stl
#include <vector>

namespace mapnik {

class MAPNIK_DECL memory_datasource : public datasource
{
    friend class memory_featureset;
public:
    memory_datasource(datasource::datasource_t type=datasource::Vector);
    virtual ~memory_datasource();
    void push(feature_ptr feature);
    datasource::datasource_t type() const;
    featureset_ptr features(const query& q) const;
    featureset_ptr features_at_point(coord2d const& pt, double tol = 0) const;
    box2d<double> envelope() const;
    boost::optional<geometry_t> get_geometry_type() const;
    layer_descriptor get_descriptor() const;
    size_t size() const;
    void clear();
private:
    std::vector<feature_ptr> features_;
    mapnik::layer_descriptor desc_;
    datasource::datasource_t type_;
};

}

#endif // MAPNIK_MEMORY_DATASOURCE_HPP
