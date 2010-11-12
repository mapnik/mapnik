/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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
//$Id$

#include <iostream>
#include <fstream>

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/unicode.hpp>

// ogr
#include "geos_featureset.hpp"
#include "geos_converter.hpp"

using std::clog;
using std::endl;

using mapnik::query;
using mapnik::box2d;
using mapnik::coord2d;
using mapnik::CoordTransform;
using mapnik::Feature;
using mapnik::feature_ptr;
using mapnik::geometry_utils;
using mapnik::transcoder;


geos_featureset::geos_featureset(GEOSGeometry* geometry,
                                 GEOSGeometry* extent,
                                 const std::string& encoding,
                                 const bool multiple_geometries)
   : geometry_(geometry),
     tr_(new transcoder(encoding)),
     extent_(extent),
     multiple_geometries_(multiple_geometries),
     already_rendered_(false)
{
}

geos_featureset::~geos_featureset() {}

feature_ptr geos_featureset::next()
{
    if (! already_rendered_)
    {
        already_rendered_ = true;
        
        if (GEOSisValid(geometry_) && ! GEOSisEmpty(geometry_))
        {
            //if (*extent_ != NULL && GEOSWithin(geometry_, *extent_))
            {
                feature_ptr feature(new Feature(0));

                geos_converter::convert_geometry (geometry_, feature, multiple_geometries_);
                
                return feature;
            }
        }
    }

    return feature_ptr();
}

