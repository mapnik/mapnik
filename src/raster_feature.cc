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

//$Id: raster_feature.cc 58 2004-10-31 16:21:26Z artem $

#include "raster_feature.hh"

namespace mapnik
{
    RasterFeature::RasterFeature(int id,const RasterPtr& ras)
        : FeatureImpl(id),
	  ras_(ras) {}

    RasterFeature::~RasterFeature()
    {
    }

    bool RasterFeature::isRaster() const
    {
        return true;
    }

    const RasterPtr& RasterFeature::getRaster() const
    {
        return ras_;
    }

    geometry_ptr&  RasterFeature::getGeometry()
    {
	static geometry_ptr geom(0);
        return geom;
    }
}
