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

//$Id: postgisfs.cc 34 2005-04-04 13:27:23Z pavlenko $


#include "postgis.hpp"

using boost::lexical_cast;
using boost::bad_lexical_cast;
using std::string;

PostgisFeatureset::PostgisFeatureset(const ref_ptr<ResultSet>& rs,
				     unsigned num_attrs=0)
    : rs_(rs),
      num_attrs_(num_attrs),
      totalGeomSize_(0),
      count_(0)  {}

Feature* PostgisFeatureset::next()
{
    Feature *feature=0;
    if (rs_->next())
    { 
	const char* buf = rs_->getValue(0);
        int id = int4net(buf);
        
        int size=rs_->getFieldLength(1);
        const char *data=rs_->getValue(1);
        geometry_ptr geom=geometry_utils::from_wkb(data,size,-1);
	totalGeomSize_+=size;
	     
        if (geom)
        {
            feature=new Feature(id,geom);
	    
	    unsigned start=2;
	    for (unsigned pos=0;pos<num_attrs_;++pos)
	    {
		const char* buf=rs_->getValue(start + pos);
		int field_size = rs_->getFieldLength(start + pos);
		int oid = rs_->getTypeOID(start + pos);
		
		if (oid==23) //int4
		{
		    int val = int4net(buf);
		    feature->add_property(val);
		}
		else if (oid==21) //int2
		{
		    int val = int2net(buf);
		    feature->add_property(val);
		}
		else if (oid == 700) // float4
		{
		    float val;
		    float4net(val,buf);
		    feature->add_property((double)val);
		}
		else if (oid == 701) // float8
		{
		    double val;
		    float8net(val,buf);
		    feature->add_property(val);
		}
		else if (oid==1042 || oid==1043) //bpchar or varchar
		{
		    feature->add_property(string(buf));
		}
		else 
		{
		    feature->add_property(string("null"));
		}
	    }
            ++count_;
        }
    }
    else
    {
        rs_->close();
        std::cout << "totalGeomSize="<<totalGeomSize_<<" bytes"<<std::endl;
        std::cout << "count="<<count_<<std::endl;
    }
    return feature;
}


PostgisFeatureset::~PostgisFeatureset()
{
    rs_->close();
}
