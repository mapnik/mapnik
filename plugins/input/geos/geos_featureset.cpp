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

// stl
#include <iostream>
#include <fstream>

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/feature_factory.hpp>

#include "geos_featureset.hpp"

using mapnik::query;
using mapnik::box2d;
using mapnik::Feature;
using mapnik::feature_ptr;
using mapnik::geometry_utils;
using mapnik::transcoder;
using mapnik::feature_factory;

geos_featureset::geos_featureset(GEOSGeometry* geometry,
                                 GEOSGeometry* extent,
                                 int identifier,
                                 std::string const& field,
                                 std::string const& field_name,
                                 std::string const& encoding)
    : geometry_(geometry),
      tr_(new transcoder(encoding)),
      extent_(extent),
      identifier_(identifier),
      field_(field),
      field_name_(field_name),
      already_rendered_(false),
      ctx_(boost::make_shared<mapnik::context_type>())
{
    ctx_->push(field_name);
}

geos_featureset::~geos_featureset()
{
}

feature_ptr geos_featureset::next()
{
    if (! already_rendered_)
    {
        already_rendered_ = true;

        if (GEOSisValid(geometry_) && ! GEOSisEmpty(geometry_))
        {
            bool render_geometry = true;

            if (*extent_ != NULL && GEOSisValid(*extent_) && ! GEOSisEmpty(*extent_))
            {
                const int type = GEOSGeomTypeId(*extent_);
                render_geometry = false;

                switch (type)
                {
                case GEOS_POINT:
                    if (GEOSIntersects(*extent_, geometry_))
                    {
                        render_geometry = true;
                    }
                    break;

                case GEOS_POLYGON:
                    if (GEOSContains(*extent_, geometry_)
                        || GEOSWithin(geometry_, *extent_)
                        || GEOSEquals(geometry_, *extent_))
                    {
                        render_geometry = true;
                    }
                    break;

                default:
                    MAPNIK_LOG_DEBUG(geos) << "geos_featureset: Unknown extent geometry_type=" << type;
                    break;
                }
            }

            if (render_geometry)
            {
                geos_wkb_ptr wkb(geometry_);
                if (wkb.is_valid())
                {
                    feature_ptr feature(feature_factory::create(ctx_,identifier_));

                    if (geometry_utils::from_wkb(feature->paths(),
                                             wkb.data(),
                                             wkb.size())
                                             && field_ != "")
                    {
                        feature->put(field_name_, tr_->transcode(field_.c_str()));
                    }

                    return feature;
                }
            }
        }
    }

    return feature_ptr();
}
