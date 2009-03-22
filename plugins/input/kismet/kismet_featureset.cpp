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

#include <mapnik/global.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/envelope.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/unicode.hpp>

#include "kismet_featureset.hpp"

using namespace std;
using namespace mapnik;

kismet_featureset::kismet_featureset(const std::list<kismet_network_data> &knd_list,
                                     std::string const& encoding)
   : knd_list_(knd_list),
     tr_(new transcoder(encoding)),
     feature_id (0),
     knd_list_it(knd_list_.begin ()),
     source_("+proj=latlong +datum=WGS84")
{
    cout << "kismet_featureset::kismet_featureset()" << endl;
}

kismet_featureset::~kismet_featureset() {}

feature_ptr kismet_featureset::next()
{
    cout << "kismet_featureset::next()" << endl;
    
    cout << "create wlan feature: " << knd_list_.size () << endl;
  
    if (knd_list_it != knd_list_.end ())
    {
      const kismet_network_data &knd = *knd_list_it;
      
      feature_ptr feature(new Feature(feature_id));
      string key = "internet_access";
      string value = "wlan";
      double outMercLongitude = 0;
      double outMercLatitude = 0;
      
      wgs84ToMercator (knd.bestlon_, knd.bestlat_, outMercLongitude, outMercLatitude);
     
      mapnik::geometry2d * pt = new mapnik::point_impl;
      pt->move_to(outMercLongitude, outMercLatitude);
      feature->add_geometry(pt);
      (*feature)[key] = tr_->transcode(value.c_str ());
      
      ++feature_id;
      ++knd_list_it;
        
      return feature;
    }
    
    // returns empty object to mark end
    return feature_ptr();
}

void kismet_featureset::wgs84ToMercator (double inWGS84Longitude, double inWGS84Latitude, 
                                         double &outMercLongitude, double &outMercLatitude)
{
  projection dest ("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over"); // initialize to your Map projection
  proj_transform proj_tr (source_, dest);

  double z = 0.0;
  proj_tr.forward (inWGS84Longitude, inWGS84Latitude, z);

  outMercLongitude = inWGS84Longitude;
  outMercLatitude = inWGS84Latitude;
}
