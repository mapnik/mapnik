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
#include <mapnik/datasource.hpp>

// boost
#include <boost/variant.hpp>

#include "rasterlite_include.hpp"

typedef boost::variant<mapnik::query,mapnik::coord2d> rasterlite_query;

class rasterlite_featureset : public mapnik::Featureset
{
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
