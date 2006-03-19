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

//$Id: raster_datasource.cc 44 2005-04-22 18:53:54Z pavlenko $

#include "raster_datasource.hpp"
#include "image_reader.hpp"
#include "raster_featureset.hpp"
#include "raster_info.hpp"
#include <boost/lexical_cast.hpp>

DATASOURCE_PLUGIN(raster_datasource)

using std::clog;
using std::endl;
using boost::lexical_cast;
using boost::bad_lexical_cast;

raster_datasource::raster_datasource(const parameters& params)
    : desc_(params.get("name"))
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

const mapnik::Envelope<double>& raster_datasource::envelope() const
{
    return extent_;
}

layer_descriptor const& raster_datasource::get_descriptor() const
{
    return desc_;
}

featureset_ptr raster_datasource::features(query const& q) const
{
    raster_info info(filename_,format_,extent_);
    single_file_policy policy(info); //todo: handle different policies!
    return featureset_ptr(new raster_featureset<single_file_policy>(policy,q));
}


