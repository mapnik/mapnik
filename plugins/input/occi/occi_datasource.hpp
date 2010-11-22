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

#ifndef OCCI_DATASOURCE_HPP
#define OCCI_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>

// boost
#include <boost/shared_ptr.hpp>

// oci
#include "occi_types.hpp"

class occi_datasource : public mapnik::datasource 
{
   public:
      occi_datasource(mapnik::parameters const& params, bool bind=true);
      virtual ~occi_datasource ();
      int type() const;
      static std::string name();
      mapnik::featureset_ptr features(mapnik::query const& q) const;
      mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt) const;
      mapnik::box2d<double> envelope() const;
      mapnik::layer_descriptor get_descriptor() const;
      void bind() const;
   private:
      int type_;
      mutable std::string table_;
      mutable std::string fields_;
      mutable std::string geometry_field_;
      mutable int srid_;
      mutable bool srid_initialized_;
      mutable bool extent_initialized_;
      mutable mapnik::box2d<double> extent_;
      mutable mapnik::layer_descriptor desc_;
      int row_limit_;
      int row_prefetch_;
      mutable oracle::occi::StatelessConnectionPool* pool_;
      mutable oracle::occi::Connection* conn_;
      bool use_connection_pool_;
      bool multiple_geometries_;
      bool use_spatial_index_;
      static const std::string METADATA_TABLE;
};


#endif // OCCI_DATASOURCE_HPP
