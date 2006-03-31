/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id: postgis.hpp 44 2005-04-22 18:53:54Z pavlenko $

#ifndef POSTGIS_HPP
#define POSTGIS_HPP


#include "datasource.hpp"
#include "envelope.hpp"
#include "feature.hpp"

#include "connection_manager.hpp"
#include <boost/lexical_cast.hpp>

#include <set>

using namespace mapnik;

class postgis_datasource : public datasource
{
    static const std::string GEOMETRY_COLUMNS;
    static const std::string SPATIAL_REF_SYS;
    const std::string uri_;
    const std::string username_;
    const std::string password_;
    const std::string table_;
    std::string geometryColumn_;
    int type_;
    int srid_;
    mapnik::Envelope<double> extent_;
    layer_descriptor desc_;
    ConnectionCreator<Connection> creator_;
    static std::string name_;
public:
    static std::string name();
    int type() const;
    featureset_ptr features(const query& q) const;
    mapnik::Envelope<double> const& envelope() const;
    layer_descriptor const& get_descriptor() const;
    postgis_datasource(const parameters &params);
    ~postgis_datasource();
private:
    static std::string table_from_sql(const std::string& sql);
    postgis_datasource(const postgis_datasource&);
    postgis_datasource& operator=(const postgis_datasource&);
};

class postgis_featureset : public Featureset
{
private:
    boost::shared_ptr<ResultSet> rs_;
    unsigned num_attrs_;
    mutable int totalGeomSize_;
    mutable int count_;
public:
    postgis_featureset(const boost::shared_ptr<ResultSet>& rs,unsigned num_attrs);
    void dispose();
    feature_ptr next();
    ~postgis_featureset();
private:
    postgis_featureset(const postgis_featureset&);
    const postgis_featureset& operator=(const postgis_featureset&);
};

#endif                                            //POSTGIS_HPP
