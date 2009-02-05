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
#include "ogr_featureset.hpp"

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


class ogr_feature_ptr
{
public:
    ogr_feature_ptr (OGRFeature* const feat)
        : feat_ (feat)
    {
    }
    
    ~ogr_feature_ptr ()
    {
        if (feat_ != NULL)
            OGRFeature::DestroyFeature (feat_);
    }

    OGRFeature* operator*()
    {
        return feat_;
    }

private:
    OGRFeature* feat_;
};


ogr_featureset::ogr_featureset(OGRDataSource & dataset,
                               OGRLayer & layer,
                               std::string const& encoding,
                               bool multiple_geometries)
   : dataset_(dataset),
     layer_(layer),
     layerdef_(layer.GetLayerDefn()),
     tr_(new transcoder(encoding)),
     fidcolumn_(layer_.GetFIDColumn ()),
     multiple_geometries_(multiple_geometries)
{
    layer_.ResetReading ();
}

ogr_featureset::~ogr_featureset() {}

feature_ptr ogr_featureset::next()
{
   ogr_feature_ptr feat (layer_.GetNextFeature());
   if ((*feat) != NULL)
   {
      OGRGeometry* geom=(*feat)->GetGeometryRef();
      if (geom != NULL)
      {
          feature_ptr feature(new Feature((*feat)->GetFID()));

          convert_geometry (geom, feature);

          int fld_count = layerdef_->GetFieldCount();
          for (int i = 0; i < fld_count; i++)
          {
              OGRFieldDefn* fld = layerdef_->GetFieldDefn (i);
              OGRFieldType type_oid = fld->GetType ();
              std_string fld_name = fld->GetNameRef ();

              switch (type_oid)
              {
               case OFTInteger:
               {
                   boost::put(*feature,fld_name,(*feat)->GetFieldAsInteger (i));
                   break;
               }

               case OFTReal:
               {
                   boost::put(*feature,fld_name,(*feat)->GetFieldAsDouble (i));
                   break;
               }
                       
               case OFTString:
               case OFTWideString:     // deprecated !
               {
                   UnicodeString ustr = tr_->transcode((*feat)->GetFieldAsString (i));
                   boost::put(*feature,fld_name,ustr);
                   break;
               }

               case OFTIntegerList:
               case OFTRealList:
               case OFTStringList:
               case OFTWideStringList: // deprecated !
               {
#ifdef MAPNIK_DEBUG
                   clog << "unhandled type_oid=" << type_oid << endl;
#endif
                   break;
               }

               case OFTBinary:
               {
#ifdef MAPNIK_DEBUG
                   clog << "unhandled type_oid=" << type_oid << endl;
#endif
                   //boost::put(*feature,name,feat->GetFieldAsBinary (i, size));
                   break;
               }
                   
               case OFTDate:
               case OFTTime:
               case OFTDateTime:       // unhandled !
               {
#ifdef MAPNIK_DEBUG
                   clog << "unhandled type_oid=" << type_oid << endl;
#endif
                   break;
               }

               default: // unknown
               {
#ifdef MAPNIK_DEBUG
                   clog << "unknown type_oid=" << type_oid << endl;
#endif
                   break;
               }
              }
          }
      
          return feature;
      }
   }

   return feature_ptr();
}

void ogr_featureset::convert_geometry (OGRGeometry* geom, feature_ptr feature)
{
  switch (wkbFlatten (geom->getGeometryType()))
  {
  case wkbPoint:
      convert_point (static_cast<OGRPoint*>(geom), feature);
      break;
  case wkbLineString:
      convert_linestring (static_cast<OGRLineString*>(geom), feature);
      break;
  case wkbPolygon:
      convert_polygon (static_cast<OGRPolygon*>(geom), feature);
      break;
  case wkbMultiPoint:
      if (multiple_geometries_)
          convert_multipoint_2 (static_cast<OGRMultiPoint*>(geom), feature);
      else
          convert_multipoint (static_cast<OGRMultiPoint*>(geom), feature);
      break;
  case wkbMultiLineString:
      if (multiple_geometries_)
          convert_multilinestring_2 (static_cast<OGRMultiLineString*>(geom), feature);
      else
          convert_multilinestring (static_cast<OGRMultiLineString*>(geom), feature);

      break;
  case wkbMultiPolygon:
      if (multiple_geometries_)
          convert_multipolygon_2 (static_cast<OGRMultiPolygon*>(geom), feature);
      else
          convert_multipolygon (static_cast<OGRMultiPolygon*>(geom), feature);
      break;
  case wkbGeometryCollection:
      convert_collection (static_cast<OGRGeometryCollection*>(geom), feature);
      break;
  case wkbLinearRing:
      convert_linestring (static_cast<OGRLinearRing*>(geom), feature);
      break;
  case wkbNone:
  case wkbUnknown:
  default:
#ifdef MAPNIK_DEBUG
      clog << "unknown <ogr> geometry_type=" << wkbFlatten (geom->getGeometryType()) << endl;
#endif
      break;
  }  
}

void ogr_featureset::convert_point (OGRPoint* geom, feature_ptr feature)
{
    geometry2d* point = new point_impl;
    point->move_to (geom->getX(), geom->getY());
    feature->add_geometry (point);
}

void ogr_featureset::convert_linestring (OGRLineString* geom, feature_ptr feature)
{
    int num_points = geom->getNumPoints ();
    geometry2d * line = new line_string_impl;
    line->set_capacity (num_points);
    line->move_to (geom->getX (0), geom->getY (0));
    for (int i=1;i<num_points;++i)
    {
        line->line_to (geom->getX (i), geom->getY (i));
    }
    feature->add_geometry (line);
}

void ogr_featureset::convert_polygon (OGRPolygon* geom, feature_ptr feature)
{
    OGRLinearRing* exterior = geom->getExteriorRing ();
    int num_points = exterior->getNumPoints ();
    int num_interior = geom->getNumInteriorRings ();
    int capacity = 0;
    for (int r=0;r<num_interior;r++)
    {
        OGRLinearRing* interior = geom->getInteriorRing (r);
        capacity += interior->getNumPoints ();
    }    
    
    geometry2d * poly = new polygon_impl;
    poly->set_capacity (num_points + capacity);
    poly->move_to (exterior->getX (0), exterior->getY (0));
    for (int i=1;i<num_points;++i)
    {
        poly->line_to (exterior->getX (i), exterior->getY (i));
    }
    for (int r=0;r<num_interior;r++)
    {
        OGRLinearRing* interior = geom->getInteriorRing (r);
        num_points = interior->getNumPoints ();
        poly->move_to(interior->getX (0), interior->getY (0));
        for (int i=1;i<num_points;++i)
        {
            poly->line_to(interior->getX (i), interior->getY (i));
        }
    }
    feature->add_geometry (poly);
}

void ogr_featureset::convert_multipoint (OGRMultiPoint* geom, feature_ptr feature)
{
    int num_geometries = geom->getNumGeometries ();
    geometry2d* point = new point_impl;

    for (int i=0;i<num_geometries;i++)
    {
        OGRPoint* ogrpoint = static_cast<OGRPoint*>(geom->getGeometryRef (i));
        point->move_to (ogrpoint->getX(), ogrpoint->getY());
    }
}

void ogr_featureset::convert_multipoint_2 (OGRMultiPoint* geom, feature_ptr feature)
{
    int num_geometries = geom->getNumGeometries ();
    for (int i=0;i<num_geometries;i++)
    {
        convert_point (static_cast<OGRPoint*>(geom->getGeometryRef (i)), feature);
    }
}

void ogr_featureset::convert_multilinestring (OGRMultiLineString* geom, feature_ptr feature)
{
    int num_geometries = geom->getNumGeometries ();

    int num_points = 0;
    for (int i=0;i<num_geometries;i++)
    {
        OGRLineString* ls = static_cast<OGRLineString*>(geom->getGeometryRef (i));
        num_points += ls->getNumPoints ();
    }

    geometry2d * line = new line_string_impl;
    line->set_capacity (num_points);

    for (int i=0;i<num_geometries;i++)
    {
        OGRLineString* ls = static_cast<OGRLineString*>(geom->getGeometryRef (i));
        num_points = ls->getNumPoints ();
        line->move_to (ls->getX (0), ls->getY (0));
        for (int i=1;i<num_points;++i)
        {
            line->line_to (ls->getX (i), ls->getY (i));
        }
    }

    feature->add_geometry (line);
}

void ogr_featureset::convert_multilinestring_2 (OGRMultiLineString* geom, feature_ptr feature)
{
    int num_geometries = geom->getNumGeometries ();
    for (int i=0;i<num_geometries;i++)
    {
        convert_linestring (static_cast<OGRLineString*>(geom->getGeometryRef (i)), feature);
    }
}

void ogr_featureset::convert_multipolygon (OGRMultiPolygon* geom, feature_ptr feature)
{
    int num_geometries = geom->getNumGeometries ();

    int capacity = 0;
    for (int i=0;i<num_geometries;i++)
    {
        OGRPolygon* p = static_cast<OGRPolygon*>(geom->getGeometryRef (i));
        OGRLinearRing* exterior = p->getExteriorRing ();
        capacity += exterior->getNumPoints ();
        for (int r=0;r<p->getNumInteriorRings ();r++)
        {
            OGRLinearRing* interior = p->getInteriorRing (r);
            capacity += interior->getNumPoints ();
        }    
    }

    geometry2d * poly = new polygon_impl;
    poly->set_capacity (capacity);

    for (int i=0;i<num_geometries;i++)
    {
        OGRPolygon* p = static_cast<OGRPolygon*>(geom->getGeometryRef (i));
        OGRLinearRing* exterior = p->getExteriorRing ();
        int num_points = exterior->getNumPoints ();
        int num_interior = p->getNumInteriorRings ();
        poly->move_to (exterior->getX (0), exterior->getY (0));
        for (int i=1;i<num_points;++i)
        {
            poly->line_to (exterior->getX (i), exterior->getY (i));
        }
        for (int r=0;r<num_interior;r++)
        {
            OGRLinearRing* interior = p->getInteriorRing (r);
            num_points = interior->getNumPoints ();
            poly->move_to(interior->getX (0), interior->getY (0));
            for (int i=1;i<num_points;++i)
            {
                poly->line_to(interior->getX (i), interior->getY (i));
            }
        }
    }

    feature->add_geometry (poly);
}

void ogr_featureset::convert_multipolygon_2 (OGRMultiPolygon* geom, feature_ptr feature)
{
    int num_geometries = geom->getNumGeometries ();
    for (int i=0;i<num_geometries;i++)
    {
        convert_polygon (static_cast<OGRPolygon*>(geom->getGeometryRef (i)), feature);
    }
}

void ogr_featureset::convert_collection (OGRGeometryCollection* geom, feature_ptr feature)
{
    int num_geometries = geom->getNumGeometries ();
    for (int i=0;i<num_geometries;i++)
    {
        OGRGeometry* g = geom->getGeometryRef (i);
        if (g != NULL)
        {
            convert_geometry (g, feature);
        }
    }
}

