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

//$Id: raster_datasource.cc 68 2004-11-23 22:39:58Z artem $

#include "raster_datasource.hh"
#include "image_reader.hh"
#include "raster_featureset.hh"
#include "raster_info.hh"

DATASOURCE_PLUGIN(raster_datasource);

raster_datasource::raster_datasource(const Parameters& params)
    : extent_()
{
    filename_=params.get("file");
    format_=params.get("format");

    double lox,loy,hix,hiy;
    fromString<double>(params.get("lox"),lox);
    fromString<double>(params.get("loy"),loy);
    fromString<double>(params.get("hix"),hix);
    fromString<double>(params.get("hiy"),hiy);

    extent_=Envelope<double>(lox,loy,hix,hiy);
}


raster_datasource::~raster_datasource()
{
}


int raster_datasource::type() const
{
    return datasource::Raster;
}


std::string raster_datasource::name()
{
    return "raster";
}

bool raster_datasource::parseEnvelope(const std::string& str,Envelope<double>& envelope) 
{   
    return true;
}

const mapnik::Envelope<double>& raster_datasource::envelope() const
{
    return extent_;
}


FeaturesetPtr raster_datasource::featuresAll(const CoordTransform& t) const
{
    return FeaturesetPtr(0);
}


FeaturesetPtr raster_datasource::featuresInBox(const CoordTransform& t,
					       const mapnik::Envelope<double>& box) const
{
    RasterInfo info(filename_,format_,extent_);
    single_file_policy policy(info); //todo: handle different policies!
    return FeaturesetPtr(new RasterFeatureset<single_file_policy>(policy,box,t));
}


FeaturesetPtr raster_datasource::featuresAtPoint(const CoordTransform& t,
						 const coord2d& pt) const
{
    return FeaturesetPtr(0);
}
