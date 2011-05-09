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

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <mapnik/global.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/sql_utils.hpp>
#include <mapnik/feature_factory.hpp>

#include "postgis.hpp"
#include <sstream>

using boost::lexical_cast;
using boost::bad_lexical_cast;
using boost::trim_copy;
using mapnik::geometry_type;
using mapnik::byte;
using mapnik::geometry_utils;
using mapnik::feature_factory;

postgis_featureset::postgis_featureset(boost::shared_ptr<IResultSet> const& rs,
                                       std::string const& encoding,
                                       bool multiple_geometries,
                                       unsigned num_attrs=0)
    : rs_(rs),
      multiple_geometries_(multiple_geometries),
      num_attrs_(num_attrs),
      tr_(new transcoder(encoding)),
      totalGeomSize_(0),
      feature_id_(1)  {}

feature_ptr postgis_featureset::next()
{
    if (rs_->next())
    { 
        feature_ptr feature(feature_factory::create(feature_id_));
        ++feature_id_;

        int size = rs_->getFieldLength(0);
        const char *data = rs_->getValue(0);
        geometry_utils::from_wkb(*feature,data,size,multiple_geometries_);
        totalGeomSize_+=size;
          
        for (unsigned pos=1;pos<num_attrs_+1;++pos)
        {
           std::string name = rs_->getFieldName(pos);

           if (!rs_->isNull(pos))
           {
              const char* buf=rs_->getValue(pos);
              int oid = rs_->getTypeOID(pos);
           
              if (oid==16) //bool
              {
                 boost::put(*feature,name,buf[0] != 0);
              }
              else if (oid==23) //int4
              {
                 int val = int4net(buf);
                 boost::put(*feature,name,val);
              }
              else if (oid==21) //int2
              {
                 int val = int2net(buf);
                 boost::put(*feature,name,val);
              }
              else if (oid==20) //int8/BigInt
              {
                 int val = int8net(buf);
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
              else if (oid==25 || oid==1043) // text or varchar
              {
                 UnicodeString ustr = tr_->transcode(buf);
                 boost::put(*feature,name,ustr);
              }
              else if (oid==1042)
              {
                 UnicodeString ustr = tr_->transcode(trim_copy(string(buf)).c_str()); // bpchar
                 boost::put(*feature,name,ustr);
              }
              else if (oid == 1700) // numeric
              {
                 std::string str = mapnik::numeric2string(buf);
                 try 
                 {
                    double val = boost::lexical_cast<double>(str);
                    boost::put(*feature,name,val);
                 }
                 catch (boost::bad_lexical_cast & ex)
                 {
                    std::clog << ex.what() << "\n"; 
                 }
              }
              else 
              {
#ifdef MAPNIK_DEBUG
                 std::clog << "Postgis Plugin: uknown OID = " << oid << " FIXME " << std::endl;
#endif
              }
           }
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
