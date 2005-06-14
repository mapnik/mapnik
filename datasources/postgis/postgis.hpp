/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id: postgis.hpp 44 2005-04-22 18:53:54Z pavlenko $

#ifndef POSTGIS_HPP
#define POSTGIS_HPP


#include "mapnik.hpp"
#include "connection_manager.hpp"
#include <boost/lexical_cast.hpp>

#include <set>

using namespace mapnik;

class PostgisDatasource : public datasource
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
    PostgisDatasource(const Parameters &params);
    ~PostgisDatasource();
private:
    static std::string table_from_sql(const std::string& sql);
    PostgisDatasource(const PostgisDatasource&);
    PostgisDatasource& operator=(const PostgisDatasource&);
};

class PostgisFeatureset : public Featureset
{
private:
    ref_ptr<ResultSet> rs_;
    unsigned num_attrs_;
    mutable int totalGeomSize_;
    mutable int count_;
public:
    PostgisFeatureset(const ref_ptr<ResultSet>& rs,unsigned num_attrs);
    void dispose();
    Feature* next();
    ~PostgisFeatureset();
private:
    PostgisFeatureset(const PostgisFeatureset&);
    const PostgisFeatureset& operator=(const PostgisFeatureset&);
};

#endif                                            //POSTGIS_HPP
