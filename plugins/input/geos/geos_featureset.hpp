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

#ifndef GEOS_FEATURESET_HPP
#define GEOS_FEATURESET_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/geom_util.hpp>

// boost
#include <boost/scoped_ptr.hpp>

// geos
#include <geos_c.h>

#include "geos_feature_ptr.hpp"

class geos_featureset : public mapnik::Featureset
{
public:
    geos_featureset(GEOSGeometry* geometry,
                    GEOSGeometry* extent,
                    int identifier,
                    std::string const& field,
                    std::string const& field_name,
                    std::string const& encoding);
    virtual ~geos_featureset();
    mapnik::feature_ptr next();

private:
    GEOSGeometry* geometry_;
    boost::scoped_ptr<mapnik::transcoder> tr_;
    geos_feature_ptr extent_;
    int identifier_;
    std::string field_;
    std::string field_name_;
    bool already_rendered_;
    mapnik::context_ptr ctx_;

    geos_featureset(const geos_featureset&);
    const geos_featureset& operator=(const geos_featureset&);
};

#endif // GEOS_FEATURESET_HPP
