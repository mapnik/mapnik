/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef OSM_FS_HH
#define OSM_FS_HH

// stl
#include <set>

// boost


// mapnik
#include <mapnik/geom_util.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/query.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/datasource.hpp>

#include "osm.h"

using mapnik::Featureset;
using mapnik::box2d;
using mapnik::feature_ptr;
using mapnik::transcoder;

template <typename filterT>
class osm_featureset : public Featureset
{
public:
    osm_featureset(const filterT& filter,
                   osm_dataset* dataset,
                   const std::set<std::string>& attribute_names,
                   std::string const& encoding);
    virtual ~osm_featureset();
    feature_ptr next();

private:
    filterT filter_;
    box2d<double> query_ext_;
    const std::unique_ptr<transcoder> tr_;
    std::vector<int> attr_ids_;
    mutable box2d<double> feature_ext_;
    mutable int total_geom_size;
    osm_dataset *dataset_;
    std::set<std::string> attribute_names_;
    mapnik::context_ptr ctx_;

    osm_featureset(const osm_featureset&);
    const osm_featureset& operator=(const osm_featureset&);
};

#endif // OSM_FS_HH
