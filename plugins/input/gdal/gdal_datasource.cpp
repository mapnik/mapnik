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
// $Id$

#include "gdal_datasource.hpp"
#include "gdal_featureset.hpp"


using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(gdal_datasource)

using mapnik::Envelope;
using mapnik::coord2d;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::datasource_exception;

gdal_datasource::gdal_datasource(parameters const& params)
   : datasource(params),
     extent_(),
     desc_(*params.get<std::string>("type"),"utf-8")
{
   GDALAllRegister();
   boost::optional<std::string> file = params.get<std::string>("file");
   if (!file) throw datasource_exception("missing <file> parameter");

   boost::optional<std::string> base = params.get<std::string>("base");
   if (base)
      dataset_name_ = *base + "/" + *file;
   else
      dataset_name_ = *file;

   dataset_ = boost::shared_ptr<GDALDataset>(reinterpret_cast<GDALDataset*>(GDALOpenShared((dataset_name_).c_str(),GA_ReadOnly)));
   if (!dataset_) throw datasource_exception(CPLGetLastErrorMsg());

   double tr[6];
   dataset_->GetGeoTransform(tr);
   double x0 = tr[0];
   double y0 = tr[3];
   double x1 = tr[0] + dataset_->GetRasterXSize()*tr[1] + dataset_->GetRasterYSize()*tr[2];
   double y1 = tr[3] + dataset_->GetRasterXSize()*tr[4] + dataset_->GetRasterYSize()*tr[5];
   extent_.init(x0,y0,x1,y1);
}

gdal_datasource::~gdal_datasource() {}

int gdal_datasource::type() const
{
   return datasource::Raster;
}

std::string gdal_datasource::name()
{
   return "gdal";
}

Envelope<double> gdal_datasource::envelope() const
{
   return extent_;
}

layer_descriptor gdal_datasource::get_descriptor() const
{
   return desc_;
}

featureset_ptr gdal_datasource::features(query const& q) const
{
   if (dataset_)
   {
      return featureset_ptr(new gdal_featureset(*dataset_, q));
   }
   return featureset_ptr();
}

featureset_ptr gdal_datasource::features_at_point(coord2d const& pt) const
{
   return featureset_ptr();
}

