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

//$Id$

#ifndef POSTGIS_HH
#define POSTGIS_HH


#include "mapnik.hh"
#include "connection_manager.hh"

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
    ConnectionCreator<Connection> creator_;
public:
    static std::string name();
    int type() const;
    FeaturesetPtr features(const query& q) const;
    const mapnik::Envelope<double>& envelope() const;
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
    mutable int totalGeomSize_;
    mutable int count_;
public:
    PostgisFeatureset(const ref_ptr<ResultSet>& rs);
    void dispose();
    Feature* next();
    ~PostgisFeatureset();
private:
    PostgisFeatureset(const PostgisFeatureset&);
    const PostgisFeatureset& operator=(const PostgisFeatureset&);
};

#endif                                            //POSTGIS_HH
