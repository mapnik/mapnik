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

//$Id: postgisfs.cc 66 2004-11-16 09:25:15Z artem $


#include "postgis.hh"

PostgisFeatureset::PostgisFeatureset(const ref_ptr<ResultSet>& rs,const CoordTransform& t)
    : rs_(rs),
      t_(t),
      totalGeomSize_(0),
      count_(0)
{
}


Feature* PostgisFeatureset::next()
{
    VectorFeature *feature=0;
    if (rs_->next())
    {
        // read gid,srid,geometry and create feature
        int id=atoi(rs_->getValue("gid"));
        int size=rs_->getFieldLength("geom");
        const char *data=rs_->getValue("geom");
        geometry_ptr geom=geometry_utils::from_wkb(data,size,-1);

        if (geom)
        {
            feature=new VectorFeature(id,geom);
            totalGeomSize_+=size;
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
