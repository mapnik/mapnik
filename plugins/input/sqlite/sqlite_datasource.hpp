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

#ifndef MAPNIK_SQLITE_DATASOURCE_HPP
#define MAPNIK_SQLITE_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp> 

// boost
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

// sqlite
#include "sqlite_connection.hpp"


class sqlite_datasource : public mapnik::datasource 
{
public:
    sqlite_datasource(mapnik::parameters const& params, bool bind = true);
    virtual ~sqlite_datasource ();
    int type() const;
    static std::string name();
    mapnik::featureset_ptr features(mapnik::query const& q) const;
    mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt) const;
    mapnik::box2d<double> envelope() const;
    mapnik::layer_descriptor get_descriptor() const;
    void bind() const;

private:
    mutable mapnik::box2d<double> extent_;
    mutable bool extent_initialized_;
    int type_;
    mutable std::string dataset_name_;
    mutable boost::shared_ptr<sqlite_connection> dataset_;
    std::string table_;
    std::string fields_;
    std::string metadata_;
    mutable std::string geometry_table_;
    mutable std::string geometry_field_;
    mutable std::string index_table_;
    mutable std::string key_field_;
    mutable int row_offset_;
    mutable int row_limit_;
    mutable mapnik::layer_descriptor desc_;
    mutable mapnik::wkbFormat format_;
    mutable bool multiple_geometries_;
    mutable bool use_spatial_index_;
    mutable bool has_spatial_index_;
    mutable bool using_subquery_;
    mutable std::vector<std::string> init_statements_;

    // Fill init_statements with any statements
    // needed to attach auxillary databases
    void parse_attachdb(std::string const& attachdb) const;
};

#endif // MAPNIK_SQLITE_DATASOURCE_HPP
