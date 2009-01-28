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
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/unicode.hpp>

#include "ogr_featureset.hpp"
#include <ogrsf_frmts.h>

using std::clog;
using std::endl;

using mapnik::query;
using mapnik::Envelope;
using mapnik::Feature;
using mapnik::feature_ptr;
using mapnik::geometry_utils;
using mapnik::CoordTransform;

ogr_featureset::ogr_featureset(OGRDataSource & dataset,
                               OGRLayer & layer,
                               OGRPolygon * extent,
                               bool multiple_geometries)
   : dataset_(dataset),
     layer_(layer),
     extent_(extent),
     fidcolumn_(layer_.GetFIDColumn ()),
     multiple_geometries_(multiple_geometries)
{
    layer_.SetSpatialFilter (extent_);
    layer_.ResetReading ();
}

ogr_featureset::~ogr_featureset() {}

feature_ptr ogr_featureset::next()
{
   OGRFeature* feat = layer_.GetNextFeature();
   if (feat != NULL)
   {
      OGRGeometry* geom=feat->GetGeometryRef();
      if (geom != NULL)
      {
          long fid=feat->GetFID(); 
          int size=geom->WkbSize ();

          feature_ptr feature(new Feature(fid));

          // XXX: here we can improve allocation
          char data[size];
          geom->exportToWkb ((OGRwkbByteOrder) endian(), reinterpret_cast<unsigned char*>(&data[0]));
          geometry_utils::from_wkb(*feature,&data[0],size,multiple_geometries_);

          OGRFeatureDefn* def = layer_.GetLayerDefn();
          for (int i = 0; i < def->GetFieldCount(); i++)
          {
              OGRFieldDefn* fld = def->GetFieldDefn (i);
              OGRFieldType type_oid = fld->GetType ();
              std_string name = fld->GetNameRef ();

              switch (type_oid)
              {
               case OFTInteger:
               // case OFTIntegerList: // TODO
                   boost::put(*feature,name,feat->GetFieldAsInteger (i));
                   break;

               case OFTReal:
               //case OFTRealList: // TODO
                   boost::put(*feature,name,feat->GetFieldAsDouble (i));
                   break;
                       
               case OFTString:
               //case OFTStringList: // TODO
                   boost::put(*feature,name,feat->GetFieldAsString (i));
                   break;
                  
               case OFTBinary:
#if 0
                   boost::put(*feature,name,feat->GetFieldAsBinary (i, size));
#endif
                   break;
                   
               case OFTDate:
               case OFTTime:
               case OFTDateTime: // unhandled !
#ifdef MAPNIK_DEBUG
                   clog << "unhandled type_oid="<<type_oid<<endl;
#endif
                   break;

               case OFTWideString:
               case OFTWideStringList: // deprecated !
#ifdef MAPNIK_DEBUG
                   //boost::put(*feature,name,tr_->transcode(feat->GetFieldAsString (i)));
                   clog << "deprecated type_oid="<<type_oid<<endl;
#endif
                   break;

               default: // unknown
#ifdef MAPNIK_DEBUG
                   clog << "unknown type_oid="<<type_oid<<endl;
#endif
                   break;
              }
          }
          
          OGRFeature::DestroyFeature (feat);
      
          return feature;
      }

      OGRFeature::DestroyFeature (feat);
   }

   return feature_ptr();
}

int ogr_featureset::endian()
{
    const int t = 1;
    return (*(char*)&t == 0) ? wkbXDR : wkbNDR;
}

/*
  OGRGeometry* geom = feat->GetGeometryRef();
  if (geom != NULL)
  {
      switch (wkbFlatten (geom->getGeometryType()))
      {
          case wkbPoint: {
              OGRPoint *poPoint = (OGRPoint *) geom;
              printf( "%.3f,%3.f\n", poPoint->getX(), poPoint->getY() );
              break;
          }
          case wkbLineString: {
              OGRLineString *poLineString = (OGRLineString *) geom;
              break;
          }
          case wkbPolygon: {
              OGRPolygon *poPolygon = (OGRPolygon *) geom;
              break;
          }
          case wkbMultiPoint: {
              OGRMultiPoint *poMultiPoint = (OGRMultiPoint *) geom;
              break;
          }
          case wkbMultiLineString: {
              OGRMultiLineString *poMultiLineString = (OGRMultiLineString *) geom;
              break;
          }
          case wkbMultiPolygon: {
              OGRMultiPolygon *poMultiPolygon = (OGRMultiPolygon *) geom;
              break;
          }
          case wkbGeometryCollection: {
              OGRGeometryCollection *poGeometryCollection = (OGRGeometryCollection *) geom;
              break;
          }
          case wkbLinearRing: {
              OGRLinearRing *poLinearRing = (OGRLinearRing *) geom;
              break;
          }
          case wkbPoint25D:
          case wkbLineString25D:
          case wkbPolygon25D:
          case wkbMultiPoint25D:
          case wkbMultiLineString25D:
          case wkbMultiPolygon25D:
          case wkbGeometryCollection25D: {
              break;
          }
          case wkbNone:
          case wkbUnknown:
          default: {
              break;
          }
      }
  }
*/

