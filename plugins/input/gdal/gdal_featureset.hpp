/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2007 Artem Pavlenko
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

#ifndef GDAL_FEATURESET_HPP
#define GDAL_FEATURESET_HPP

#include <mapnik/datasource.hpp>
#include <boost/variant.hpp>

class GDALDataset;
class GDALRasterBand;

typedef boost::variant<mapnik::query,mapnik::coord2d> gdal_query;

class gdal_featureset : public mapnik::Featureset
{
   public:
      
      gdal_featureset(GDALDataset & dataset, int band, gdal_query q);
      virtual ~gdal_featureset();
      mapnik::feature_ptr next();
   private:
      mapnik::feature_ptr get_feature(mapnik::query const& q);
      mapnik::feature_ptr get_feature_at_point(mapnik::coord2d const& p);
      void get_overview_meta(GDALRasterBand * band);
      GDALDataset & dataset_;
      int band_;
      gdal_query gquery_;
      bool first_;
};

#endif // GDAL_FEATURESET_HPP
