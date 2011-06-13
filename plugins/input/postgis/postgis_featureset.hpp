
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

#ifndef POSTGIS_FEATURESET_HPP
#define POSTGIS_FEATURESET_HPP

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/feature.hpp>

// boost

#include <boost/scoped_ptr.hpp>

using mapnik::Featureset;
using mapnik::box2d;
using mapnik::feature_ptr;
using mapnik::transcoder;

class IResultSet;

class postgis_featureset : public mapnik::Featureset
{
private:
    boost::shared_ptr<IResultSet> rs_;
    bool multiple_geometries_;
    unsigned num_attrs_;
    boost::scoped_ptr<mapnik::transcoder> tr_;
    int totalGeomSize_;
    int feature_id_;
    bool key_field_;
public:
    postgis_featureset(boost::shared_ptr<IResultSet> const& rs,
                       std::string const& encoding,
                       bool multiple_geometries,
                       bool key_field,
                       unsigned num_attrs);
    mapnik::feature_ptr next();
    ~postgis_featureset();
private:
    postgis_featureset(const postgis_featureset&);
    const postgis_featureset& operator=(const postgis_featureset&);
};

#endif // POSTGIS_FEATURESET_HPP
