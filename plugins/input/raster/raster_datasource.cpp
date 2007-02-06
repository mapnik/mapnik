/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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
//$Id: raster_datasource.cc 44 2005-04-22 18:53:54Z pavlenko $
// boost
#include <boost/lexical_cast.hpp>
// mapnik
#include <mapnik/image_reader.hpp>

#include "raster_featureset.hpp"
#include "raster_info.hpp"

#include "raster_datasource.hpp"

DATASOURCE_PLUGIN(raster_datasource)

using std::clog;
using std::endl;
using boost::lexical_cast;
using boost::bad_lexical_cast;

raster_datasource::raster_datasource(const parameters& params)
    : datasource (params),
      desc_(params.get("name"),"utf-8")
{
    filename_=params.get("file");
    format_=params.get("format");
    
    try 
    {
        double lox=lexical_cast<double>(params.get("lox"));
        double loy=lexical_cast<double>(params.get("loy"));
        double hix=lexical_cast<double>(params.get("hix"));
        double hiy=lexical_cast<double>(params.get("hiy"));
        extent_.init(lox,loy,hix,hiy);
    }
    catch (bad_lexical_cast& ex)
    {
        clog << ex.what() << endl;
    }  
}


raster_datasource::~raster_datasource()
{
}

int raster_datasource::type() const
{
    return datasource::Raster;
}

std::string raster_datasource::name_="raster";
std::string raster_datasource::name()
{
    return name_;
}

mapnik::Envelope<double> raster_datasource::envelope() const
{
    return extent_;
}

layer_descriptor raster_datasource::get_descriptor() const
{
    return desc_;
}

featureset_ptr raster_datasource::features(query const& q) const
{
    raster_info info(filename_,format_,extent_);
    single_file_policy policy(info); //todo: handle different policies!
    return featureset_ptr(new raster_featureset<single_file_policy>(policy,q));
}


featureset_ptr raster_datasource::features_at_point(coord2d const&) const
{
    return featureset_ptr();
}

