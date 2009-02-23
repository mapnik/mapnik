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

// ogr
#include "sqlite_featureset.hpp"

using std::clog;
using std::endl;

using mapnik::query;
using mapnik::Envelope;
using mapnik::CoordTransform;
using mapnik::Feature;
using mapnik::feature_ptr;
using mapnik::point_impl;
using mapnik::line_string_impl;
using mapnik::polygon_impl;
using mapnik::geometry2d;
using mapnik::geometry_utils;
using mapnik::transcoder;

sqlite_featureset::sqlite_featureset(boost::shared_ptr<sqlite_resultset> rs,
                                     std::string const& encoding,
                                     mapnik::wkbFormat format,
                                     bool multiple_geometries)
   : rs_(rs),
     tr_(new transcoder(encoding)),
     format_(format),
     multiple_geometries_(multiple_geometries)
{
}

sqlite_featureset::~sqlite_featureset() {}

feature_ptr sqlite_featureset::next()
{
    if (rs_->is_valid () && rs_->step_next ())
    {
        int size;
        const char* data = (const char *) rs_->column_blob (0, size);
        int feature_id = rs_->column_integer (1);   

#ifdef MAPNIK_DEBUG
        // clog << "feature_oid=" << feature_id << endl;
#endif

        feature_ptr feature(new Feature(feature_id));
        geometry_utils::from_wkb(*feature,data,size,multiple_geometries_,format_);
        
        for (int i = 2; i < rs_->column_count (); ++i)
        {
           const int type_oid = rs_->column_type (i);
           const char* fld_name = rs_->column_name (i);
           
           switch (type_oid)
           {
              case SQLITE_INTEGER:
              {
                 boost::put(*feature,fld_name,rs_->column_integer (i));
                 break;
              }
              
              case SQLITE_FLOAT:
              {
                 boost::put(*feature,fld_name,rs_->column_double (i));
                 break;
              }
              
              case SQLITE_TEXT:
              {
                 UnicodeString ustr = tr_->transcode (rs_->column_text (i));
                 boost::put(*feature,fld_name,ustr);
                 break;
              }
              
              case SQLITE_BLOB:
              case SQLITE_NULL:
                 break;
                 
              default:
#ifdef MAPNIK_DEBUG
                 clog << "unhandled type_oid=" << type_oid << endl;
#endif
                 break;
           }
        }

        return feature;
    }

    return feature_ptr();
}

