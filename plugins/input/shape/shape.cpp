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

#include "shape.hpp"
#include "shape_featureset.hpp"
#include "shape_index_featureset.hpp"
#include "geom_util.hpp"

#include <iostream>
#include <stdexcept>

DATASOURCE_PLUGIN(shape_datasource)

shape_datasource::shape_datasource(const parameters &params)
    : shape_name_(params.get("file")),
      type_(datasource::Vector),
      file_length_(0),
      indexed_(false),
      desc_(params.get("name"))
{
    try
    {
        shape_io shape(shape_name_);
        init(shape);
	for (int i=0;i<shape.dbf().num_fields();++i)
	{
	    field_descriptor const& fd=shape.dbf().descriptor(i);
	    std::string fld_name=fd.name_;

	    switch (fd.type_)
	    {
	    case 'C':
	    case 'D':
	    case 'M':
	    case 'L':		
		desc_.add_descriptor(attribute_descriptor(fld_name,String));
		break;
	    case 'N':
	    case 'F':
		{
		    if (fd.dec_>0)
		    {   
			desc_.add_descriptor(attribute_descriptor(fld_name,Double,false,8));
		    }
		    else
		    {
			desc_.add_descriptor(attribute_descriptor(fld_name,Integer,false,4));
		    }
		    break;
		}
	    default:
		//
		std::cout << "uknown type "<<fd.type_<<"\n";
		break;
		
	    }
	}
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

std::string shape_datasource::name_="shape";

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

layer_descriptor const& shape_datasource::get_descriptor() const
{
    return desc_;
}

std::string shape_datasource::name()
{
    return name_;
}

featureset_ptr shape_datasource::features(const query& q) const
{
    filter_in_box filter(q.get_bbox());
    if (indexed_)
    {
	return featureset_ptr(new shape_index_featureset<filter_in_box>(filter,shape_name_,q.property_names()));
    }
    return featureset_ptr(new shape_featureset<filter_in_box>(filter,shape_name_,q.property_names(),file_length_));
}

const Envelope<double>& shape_datasource::envelope() const
{
    return extent_;
}
