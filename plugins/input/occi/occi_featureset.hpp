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

#ifndef OCCI_FEATURESET_HPP
#define OCCI_FEATURESET_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/unicode.hpp> 

// boost
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

// oci
#include "occi_types.hpp"
  
class occi_featureset : public mapnik::Featureset
{
   public:
      occi_featureset(oracle::occi::StatelessConnectionPool * pool,
                      std::string const& sqlstring,
                      std::string const& encoding,
                      bool multiple_geometries,
                      unsigned prefetch_rows,
                      unsigned num_attrs);
      virtual ~occi_featureset();
      mapnik::feature_ptr next();
   private:
      void convert_geometry (SDOGeometry* geom, mapnik::feature_ptr feature);
      void convert_point (SDOGeometry* geom, mapnik::feature_ptr feature, int dims);
      void convert_linestring (SDOGeometry* geom, mapnik::feature_ptr feature, int dims);
      void convert_polygon (SDOGeometry* geom, mapnik::feature_ptr feature, int dims);
      void convert_multipoint (SDOGeometry* geom, mapnik::feature_ptr feature, int dims);
      //void convert_multipoint_2 (SDOGeometry* geom, mapnik::feature_ptr feature, int dims);
      void convert_multilinestring (SDOGeometry* geom, mapnik::feature_ptr feature, int dims);
      //void convert_multilinestring_2 (SDOGeometry* geom, mapnik::feature_ptr feature, int dims);
      void convert_multipolygon (SDOGeometry* geom, mapnik::feature_ptr feature, int dims);
      //void convert_multipolygon_2 (SDOGeometry* geom, mapnik::feature_ptr feature, int dims);
      //void convert_collection (SDOGeometry* geom, mapnik::feature_ptr feature, int dims);
      void fill_geometry2d (mapnik::geometry2d * geom,
                             const std::vector<oracle::occi::Number>& elem_info,
                             const std::vector<oracle::occi::Number>& ordinates,
                             const int dimensions,
                             const bool is_point_geom);
      void fill_geometry2d (mapnik::geometry2d * geom,
                             const int real_offset,
                             const int next_offset,
                             const std::vector<oracle::occi::Number>& ordinates,
                             const int dimensions,
                             const bool is_point_geom);
      occi_connection_ptr conn_;
      oracle::occi::ResultSet* rs_;
      boost::scoped_ptr<mapnik::transcoder> tr_;
      const char* fidcolumn_;
      bool multiple_geometries_;
      unsigned num_attrs_;
      mutable int count_;
};

#endif // OCCI_FEATURESET_HPP
