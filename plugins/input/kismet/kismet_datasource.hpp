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

#ifndef KISMET_DATASOURCE_HPP
#define KISMET_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>
#include <mapnik/query.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/feature_layer_desc.hpp>

// boost
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

// stl
#include <list>
#include <vector>
#include <string>

#include "kismet_types.hpp"

class kismet_datasource : public mapnik::datasource
{
public:
    kismet_datasource(mapnik::parameters const& params, bool bind = true);
    virtual ~kismet_datasource ();
    datasource::datasource_t type() const;
    static const char * name();
    mapnik::featureset_ptr features(mapnik::query const& q) const;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt, double tol = 0) const;
    mapnik::box2d<double> envelope() const;
    boost::optional<mapnik::datasource::geometry_t> get_geometry_type() const;
    mapnik::layer_descriptor get_descriptor() const;
    void bind() const;

private:
    void run (std::string const& host, const unsigned int port);

    mapnik::box2d<double> extent_;
    bool extent_initialized_;
    std::string host_;
    unsigned int port_;
    mapnik::datasource::datasource_t type_;
    std::string srs_;
    mutable mapnik::layer_descriptor desc_;
    boost::shared_ptr<boost::thread> kismet_thread;
};

#endif // KISMET_DATASOURCE_HPP
