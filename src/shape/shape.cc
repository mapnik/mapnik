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

#include "shape.hh"
#include "shape_featureset.hh"
#include "shape_index_featureset.hh"

#include <iostream>
#include <stdexcept>

DATASOURCE_PLUGIN(shape_datasource);

shape_datasource::shape_datasource(const Parameters &params)
    : shape_name_(params.get("file")),
      file_length_(0),
      indexed_(false)
{
    try
    {
        shape_io shape(shape_name_);
        init(shape);
    }
    catch  (datasource_exception& ex)
    {
        std::cerr<<ex.what()<<std::endl;
        throw;
    }
}


shape_datasource::~shape_datasource()
{
}


void  shape_datasource::init(shape_io& shape)
{
    //first read header from *.shp
    int file_code=shape.shp().read_xdr_integer();
    if (file_code!=9994)
    {
        //invalid
        throw datasource_exception("wrong file code");
    }
    shape.shp().skip(5*4);
    file_length_=shape.shp().read_xdr_integer();
    int version=shape.shp().read_ndr_integer();
    if (version!=1000)
    {
        //invalid version number
        throw datasource_exception("invalid version number");
    }
    int shape_type=shape.shp().read_ndr_integer();
    if (shape_type==1 || shape_type==8 || shape_type==18 || shape_type==28)
        type_=datasource::Point;
    else if (shape_type==3 || shape_type==13 || shape_type==23)
        type_=datasource::Line;
    else if (shape_type==5 || shape_type==15 || shape_type==25)
        type_=datasource::Polygon;

    shape.shp().read_envelope(extent_);
    shape.shp().skip(4*8);

    // check if we have an index file around
    std::string index_name(shape_name_+".index");
    std::ifstream file(index_name.c_str(),std::ios::in | std::ios::binary);
    if (file)
    {
	indexed_=true;
	file.close();
    }

    std::cout<<extent_<<std::endl;
    std::cout<<"file_length="<<file_length_<<std::endl;
    std::cout<<"shape_type="<<shape_type<<std::endl;
}


int shape_datasource::type() const
{
    return type_;
}


std::string shape_datasource::name()
{
    return "shape";
}

FeaturesetPtr shape_datasource::features(const query& q) const
{
    filter_in_box filter(q.get_bbox());
    if (indexed_)
    {
	return FeaturesetPtr(new shape_index_featureset<filter_in_box>(filter,shape_name_,q.property_names()));
    }
    return FeaturesetPtr(new shape_featureset<filter_in_box>(filter,shape_name_,file_length_));
}

const Envelope<double>& shape_datasource::envelope() const
{
    return extent_;
}
