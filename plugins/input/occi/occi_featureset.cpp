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
#include "occi_featureset.hpp"

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
using mapnik::datasource_exception;

using oracle::occi::Connection;
using oracle::occi::Statement;
using oracle::occi::ResultSet;
using oracle::occi::StatelessConnectionPool;
using oracle::occi::MetaData;
using oracle::occi::SQLException;
using oracle::occi::Type;
using oracle::occi::Number;

occi_featureset::occi_featureset(StatelessConnectionPool * pool,
                                 std::string const& sqlstring,
                                 std::string const& encoding,
                                 bool multiple_geometries,
                                 unsigned prefetch_rows,
                                 unsigned num_attrs)
   : conn_(pool),
     tr_(new transcoder(encoding)),
     multiple_geometries_(multiple_geometries),
     num_attrs_(num_attrs),
     count_(0)
{
    try
    {
        rs_ = conn_.execute_query (sqlstring, prefetch_rows);
    }
    catch (SQLException &ex)
    {
        throw datasource_exception(ex.getMessage());
    }
}

occi_featureset::~occi_featureset()
{
}

feature_ptr occi_featureset::next()
{
    if (rs_ && rs_->next())
    {
        feature_ptr feature(new Feature(count_));

        boost::scoped_ptr<SDOGeometry> geom (dynamic_cast<SDOGeometry*> (rs_->getObject(1)));
        if (geom.get())
        {
            convert_geometry (geom.get(), feature);
        }

        std::vector<MetaData> listOfColumns = rs_->getColumnListMetaData();

        for (unsigned int i=1;i<listOfColumns.size();++i)
        {
           MetaData columnObj = listOfColumns[i];

           std::string fld_name = columnObj.getString(MetaData::ATTR_NAME);
           int type_oid = columnObj.getInt(MetaData::ATTR_DATA_TYPE);

#if 0
           int type_code = columnObj.getInt(MetaData::ATTR_TYPECODE);
           if (type_code == OCCI_TYPECODE_OBJECT)
           {
               continue;
           }
#endif

           switch (type_oid)
           {
           case oracle::occi::OCCIINT:
           case oracle::occi::OCCIUNSIGNED_INT:
           {
              boost::put(*feature,fld_name,rs_->getInt (i + 1));
              break;
           }
           
           case oracle::occi::OCCIFLOAT:
           case oracle::occi::OCCIBFLOAT:
           case oracle::occi::OCCIDOUBLE:
           case oracle::occi::OCCIBDOUBLE:
           case oracle::occi::OCCINUMBER:
           case oracle::occi::OCCI_SQLT_NUM:
           {
              boost::put(*feature,fld_name,rs_->getDouble (i + 1));
              break;
           }

           case oracle::occi::OCCICHAR:
           case oracle::occi::OCCISTRING:
           case oracle::occi::OCCI_SQLT_AFC:
           case oracle::occi::OCCI_SQLT_AVC:
           case oracle::occi::OCCI_SQLT_CHR:
           case oracle::occi::OCCI_SQLT_LVC:
           case oracle::occi::OCCI_SQLT_STR:
           case oracle::occi::OCCI_SQLT_VCS:
           case oracle::occi::OCCI_SQLT_VNU:
           case oracle::occi::OCCI_SQLT_VBI:
           case oracle::occi::OCCI_SQLT_VST:
           case oracle::occi::OCCI_SQLT_RDD:
           {
              UnicodeString ustr = tr_->transcode (rs_->getString (i + 1).c_str());
              boost::put(*feature,fld_name,ustr);
              break;
           }
           
           case oracle::occi::OCCIDATE:
           case oracle::occi::OCCITIMESTAMP:
           case oracle::occi::OCCI_SQLT_DAT:
           case oracle::occi::OCCI_SQLT_TIMESTAMP:
           case oracle::occi::OCCI_SQLT_TIMESTAMP_LTZ:
           case oracle::occi::OCCI_SQLT_TIMESTAMP_TZ:
           case oracle::occi::OCCIPOBJECT:
           {
#ifdef MAPNIK_DEBUG
              clog << "unsupported type_oid="<<type_oid<<endl;
#endif
              break;
           }

           default: // shouldn't get here
           {
#ifdef MAPNIK_DEBUG
              clog << "unknown type_oid="<<type_oid<<endl;
#endif
              break;
           }
           }	  
        }
        
        ++count_;
        return feature;
    }

    return feature_ptr();
}


void occi_featureset::convert_geometry (SDOGeometry* geom, feature_ptr feature)
{
   int gtype = (int) geom->getSdo_gtype();
   int dimensions = gtype / 1000;
   int lrsvalue = (gtype - dimensions * 1000) / 100;
   int geomtype = (gtype - dimensions * 1000 - lrsvalue * 100); 

#if 0
   clog << "-----------Geometry Object ------------" << endl;
   clog << "SDO GTYPE = " << gtype << endl;
   clog << "SDO DIMENSIONS = " << dimensions << endl;
   clog << "SDO LRS = " << lrsvalue << endl;
   clog << "SDO GEOMETRY TYPE = " << geomtype << endl;

   Number sdo_srid = geom->getSdo_srid();
   if (sdo_srid.isNull())
      clog << "SDO SRID = " << "Null" << endl;
   else
      clog << "SDO SRID = " << (int) sdo_srid << endl;
#endif

   switch (geomtype)
   {
   case SDO_GTYPE_POINT:
       convert_point (geom, feature, dimensions);
       break;
   case SDO_GTYPE_LINE:
       convert_linestring (geom, feature, dimensions);
       break;
   case SDO_GTYPE_POLYGON:
       convert_polygon (geom, feature, dimensions);
       break;
   case SDO_GTYPE_MULTIPOINT:
       convert_multipoint (geom, feature, dimensions);
       break;
   case SDO_GTYPE_MULTILINE:
       convert_multilinestring (geom, feature, dimensions);
       break;
   case SDO_GTYPE_MULTIPOLYGON:
       convert_multipolygon (geom, feature, dimensions);
       break;
   case SDO_GTYPE_COLLECTION:
#ifdef MAPNIK_DEBUG
       clog << "unsupported <occi> collection" << endl;
#endif
       break;
   case SDO_GTYPE_UNKNOWN:
   default:
#ifdef MAPNIK_DEBUG
       clog << "unknown <occi> geometry_type=" << gtype << endl;
#endif
       break;
   }
}

void occi_featureset::convert_point (SDOGeometry* geom, feature_ptr feature, int dimensions)
{
    SDOPointType* sdopoint = geom->getSdo_point();
    if (sdopoint && ! sdopoint->isNull())
    {
        geometry2d* point = new point_impl;

        point->move_to (sdopoint->getX(), sdopoint->getY());

        feature->add_geometry (point);
    }
}

void occi_featureset::convert_linestring (SDOGeometry* geom, feature_ptr feature, int dimensions)
{
    const std::vector<Number>& elem_info = geom->getSdo_elem_info();
    const std::vector<Number>& ordinates = geom->getSdo_ordinates();
    int ord_size = ordinates.size();

    if (ord_size >= dimensions)
    {
        geometry2d * line = new line_string_impl;
        line->set_capacity (ord_size);

        fill_geometry2d (line, elem_info, ordinates, dimensions, false);
        
        feature->add_geometry (line);
    }
}

void occi_featureset::convert_polygon (SDOGeometry* geom, feature_ptr feature, int dimensions)
{
    const std::vector<Number>& elem_info = geom->getSdo_elem_info();
    const std::vector<Number>& ordinates = geom->getSdo_ordinates();
    int ord_size = ordinates.size();

    if (ord_size >= dimensions)
    {
        geometry2d * poly = new polygon_impl;
        poly->set_capacity (ord_size);

        fill_geometry2d (poly, elem_info, ordinates, dimensions, false);

        feature->add_geometry (poly);
    }
}

void occi_featureset::convert_multipoint (SDOGeometry* geom, feature_ptr feature, int dimensions)
{
    const std::vector<Number>& elem_info = geom->getSdo_elem_info();
    const std::vector<Number>& ordinates = geom->getSdo_ordinates();
    int ord_size = ordinates.size();

    if (ord_size >= dimensions)
    {
        geometry2d * point = new point_impl;

        fill_geometry2d (point, elem_info, ordinates, dimensions, true);

        feature->add_geometry (point);
    }
}

/*
void occi_featureset::convert_multipoint_2 (SDOGeometry* geom, feature_ptr feature, int dimensions)
{
    int num_geometries = geom->getNumGeometries ();
    for (int i=0;i<num_geometries;i++)
    {
        convert_point (static_cast<OGRPoint*>(geom->getGeometryRef (i)), feature);
    }
}
*/

void occi_featureset::convert_multilinestring (SDOGeometry* geom, feature_ptr feature, int dimensions)
{
    const std::vector<Number>& elem_info = geom->getSdo_elem_info();
    const std::vector<Number>& ordinates = geom->getSdo_ordinates();
    int ord_size = ordinates.size();

    if (ord_size >= dimensions)
    {
        geometry2d * line = new line_string_impl;
        line->set_capacity (ord_size);

        fill_geometry2d (line, elem_info, ordinates, dimensions, false);
        
        feature->add_geometry (line);
    }
}

/*
void occi_featureset::convert_multilinestring_2 (SDOGeometry* geom, feature_ptr feature, int dimensions)
{
    int num_geometries = geom->getNumGeometries ();
    for (int i=0;i<num_geometries;i++)
    {
        convert_linestring (static_cast<OGRLineString*>(geom->getGeometryRef (i)), feature);
    }
}
*/

void occi_featureset::convert_multipolygon (SDOGeometry* geom, feature_ptr feature, int dimensions)
{
    const std::vector<Number>& elem_info = geom->getSdo_elem_info();
    const std::vector<Number>& ordinates = geom->getSdo_ordinates();
    int ord_size = ordinates.size();

    if (ord_size >= dimensions)
    {
        geometry2d * poly = new polygon_impl;
        poly->set_capacity (ord_size);

        fill_geometry2d (poly, elem_info, ordinates, dimensions, false);

        feature->add_geometry (poly);
    }
}

/*
void occi_featureset::convert_multipolygon_2 (SDOGeometry* geom, feature_ptr feature, int dimensions)
{
    int num_geometries = geom->getNumGeometries ();
    for (int i=0;i<num_geometries;i++)
    {
        convert_polygon (static_cast<OGRPolygon*>(geom->getGeometryRef (i)), feature);
    }
}
*/

/*
void occi_featureset::convert_collection (SDOGeometry* geom, feature_ptr feature, int dimensions)
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
*/

void occi_featureset::fill_geometry2d (geometry2d * geom,
                                        const std::vector<Number>& elem_info,
                                        const std::vector<Number>& ordinates,
                                        const int dimensions,
                                        const bool is_point_geom)
{
    int elem_size = elem_info.size();
    int ord_size = ordinates.size();

    int offset, etype, interp;
    if (elem_size >= 0)
    {
        offset = elem_info [0];
        etype = elem_info [1];
        interp = elem_info [2];

        if (elem_size > SDO_ELEM_INFO_SIZE)
        {
            for (int i = SDO_ELEM_INFO_SIZE; i < elem_size; i+=3)
            {
                int next_offset = elem_info [i];
                int next_etype = elem_info [i + 1];
                int next_interp = elem_info [i + 2];
                bool is_linear_element = true;
                bool is_unknown_etype = false;

                switch (etype)
                {
                case SDO_ETYPE_POINT:
                    if (interp == SDO_INTERPRETATION_POINT) {}
                    if (interp > SDO_INTERPRETATION_POINT)  {}
                    break;
                
                case SDO_ETYPE_LINESTRING:
                    if (interp == SDO_INTERPRETATION_STRAIGHT) {}
                    if (interp == SDO_INTERPRETATION_CIRCULAR) {}
                    break;

                case SDO_ETYPE_POLYGON:
                case SDO_ETYPE_POLYGON_INTERIOR:
                    if (interp == SDO_INTERPRETATION_STRAIGHT)  {}
                    if (interp == SDO_INTERPRETATION_CIRCULAR)  {}
                    if (interp == SDO_INTERPRETATION_RECTANGLE) {}
                    if (interp == SDO_INTERPRETATION_CIRCLE)    {}
                    break;

                case SDO_ETYPE_COMPOUND_LINESTRING:
                case SDO_ETYPE_COMPOUND_POLYGON:
                case SDO_ETYPE_COMPOUND_POLYGON_INTERIOR:
                    // interp = next ETYPE to consider
                    is_linear_element = false;
                    break;

                case SDO_ETYPE_UNKNOWN:    // unknown
                default:
                    is_unknown_etype = true;
                    break;
                }

                if (is_unknown_etype)
                    break;

                if (is_linear_element)
                {
                    fill_geometry2d (geom,
                                     offset - 1,
                                     next_offset - 1,
                                     ordinates,
                                     dimensions,
                                     is_point_geom);
                }

                offset = next_offset;
                etype = next_etype;
                interp = next_interp;
            }
        }
        else
        {
            fill_geometry2d (geom,
                             offset - 1,
                             ord_size,
                             ordinates,
                             dimensions,
                             is_point_geom);
        }
    }
}

void occi_featureset::fill_geometry2d (geometry2d * geom,
                                        const int real_offset,
                                        const int next_offset,
                                        const std::vector<Number>& ordinates,
                                        const int dimensions,
                                        const bool is_point_geom)
{
    geom->move_to ((double) ordinates[real_offset], (double) ordinates[real_offset + 1]);

    if (is_point_geom)
        for (int p = real_offset + dimensions; p < next_offset; p += dimensions)
            geom->move_to ((double) ordinates[p], (double) ordinates[p + 1]);
    else
        for (int p = real_offset + dimensions; p < next_offset; p += dimensions)
            geom->line_to ((double) ordinates[p], (double) ordinates[p + 1]);

}
