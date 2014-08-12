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

#ifndef RASTERLITE_FEATURESET_HPP
#define RASTERLITE_FEATURESET_HPP

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/query.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/util/variant.hpp>

#include "rasterlite_include.hpp"

using rasterlite_query = mapnik::util::variant<mapnik::query,mapnik::coord2d>;

class rasterlite_featureset : public mapnik::Featureset
{
    struct query_dispatch : public mapnik::util::static_visitor<mapnik::feature_ptr>
    {
        query_dispatch( rasterlite_featureset & featureset)
            : featureset_(featureset) {}

        mapnik::feature_ptr operator() (mapnik::query const& q) const
        {
            return featureset_.get_feature(q);
        }

        mapnik::feature_ptr operator() (mapnik::coord2d const& p) const
        {
            return featureset_.get_feature_at_point(p);
        }

        rasterlite_featureset & featureset_;
    };

public:
    rasterlite_featureset(void* dataset,
                          rasterlite_query q);
    virtual ~rasterlite_featureset();
    mapnik::feature_ptr next();

private:
    mapnik::feature_ptr get_feature(mapnik::query const& q);
    mapnik::feature_ptr get_feature_at_point(mapnik::coord2d const& p);
    void* dataset_;
    rasterlite_query gquery_;
    bool first_;
    mapnik::context_ptr ctx_;
};

#endif // RASTERLITE_FEATURESET_HPP
