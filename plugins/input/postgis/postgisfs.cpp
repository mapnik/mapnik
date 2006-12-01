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

//$Id: postgisfs.cc 34 2005-04-04 13:27:23Z pavlenko $

#include <mapnik/global.hpp>
#include <mapnik/wkb.hpp>
#include "postgis.hpp"

using boost::lexical_cast;
using boost::bad_lexical_cast;
using std::string;

postgis_featureset::postgis_featureset(boost::shared_ptr<ResultSet> const& rs,
                                       unsigned num_attrs=0)
    : rs_(rs),
      num_attrs_(num_attrs),
      totalGeomSize_(0),
      count_(0)  {}

feature_ptr postgis_featureset::next()
{
    if (rs_->next())
    { 
        feature_ptr feature(new Feature(count_));
        int size=rs_->getFieldLength(0);
        const char *data=rs_->getValue(0);
        geometry_ptr geom=geometry_utils::from_wkb(data,size,-1);
        totalGeomSize_+=size;
	     
        if (geom)
        {
            feature->set_geometry(geom);
            
            for (unsigned pos=1;pos<num_attrs_+1;++pos)
            {
                std::string name = rs_->getFieldName(pos);
                const char* buf=rs_->getValue(pos);
                int oid = rs_->getTypeOID(pos);
                
                if (oid==23) //int4
                {
                    int val = int4net(buf);
                    boost::put(*feature,name,val);
                }
                else if (oid==21) //int2
                {
                    int val = int2net(buf);
                    boost::put(*feature,name,val);
                }
                else if (oid == 700) // float4
                {
                    float val;
                    float4net(val,buf);
                    boost::put(*feature,name,val);
                }
                else if (oid == 701) // float8
                {
                    double val;
                    float8net(val,buf);
                    boost::put(*feature,name,val);
                }
                else if (oid==25 || oid==1042 || oid==1043) // text or bpchar or varchar
                {
                    boost::put(*feature,name,buf);
                }
                else 
                {
#ifdef MAPNIK_DEBUG
                    std::clog << "uknown OID = " << oid << " FIXME \n";
#endif
                    //boost::put(*feature,name,0);
                }
            }
            ++count_;
        }
        return feature;
    }
    else
    {
        rs_->close();
        return feature_ptr();
    }
}


postgis_featureset::~postgis_featureset()
{
    rs_->close();
}
