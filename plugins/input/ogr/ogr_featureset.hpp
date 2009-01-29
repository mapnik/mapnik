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

#ifndef OGR_FEATURESET_HPP
#define OGR_FEATURESET_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/unicode.hpp> 

// boost
#include <boost/scoped_ptr.hpp>

// ogr
#include <ogrsf_frmts.h>
  
class ogr_featureset : public mapnik::Featureset
{
   public:
      ogr_featureset(OGRDataSource & dataset,
                     OGRLayer & layer,
                     std::string const& encoding,
                     bool multiple_geometries);
      virtual ~ogr_featureset();
      mapnik::feature_ptr next();
   private:
      int endian();
      void convert_geometry (OGRGeometry* geom, mapnik::feature_ptr feature);
      void convert_point (OGRPoint* geom, mapnik::feature_ptr feature);
      void convert_linestring (OGRLineString* geom, mapnik::feature_ptr feature);
      void convert_polygon (OGRPolygon* geom, mapnik::feature_ptr feature);
      void convert_multipoint (OGRMultiPoint* geom, mapnik::feature_ptr feature);
      void convert_multipoint_2 (OGRMultiPoint* geom, mapnik::feature_ptr feature);
      void convert_multilinestring (OGRMultiLineString* geom, mapnik::feature_ptr feature);
      void convert_multilinestring_2 (OGRMultiLineString* geom, mapnik::feature_ptr feature);
      void convert_multipolygon (OGRMultiPolygon* geom, mapnik::feature_ptr feature);
      void convert_multipolygon_2 (OGRMultiPolygon* geom, mapnik::feature_ptr feature);
      void convert_collection (OGRGeometryCollection* geom, mapnik::feature_ptr feature);
      OGRDataSource & dataset_;
      OGRLayer & layer_;
      boost::scoped_ptr<mapnik::transcoder> tr_;
      const char* fidcolumn_;
      bool multiple_geometries_;
};

#endif // OGR_FEATURESET_HPP
