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

#ifndef SQLITE_DATASOURCE_HPP
#define SQLITE_DATASOURCE_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

// sqlite
#include "sqlite_types.hpp"

class sqlite_datasource : public mapnik::datasource 
{
   public:
      sqlite_datasource(mapnik::parameters const& params);
      virtual ~sqlite_datasource ();
      int type() const;
      static std::string name();
      mapnik::featureset_ptr features(mapnik::query const& q) const;
      mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt) const;
      mapnik::Envelope<double> envelope() const;
      mapnik::layer_descriptor get_descriptor() const;
   private:
      static const std::string name_;
      mapnik::Envelope<double> extent_;
      mutable bool extent_initialized_;
      int type_;
      sqlite_connection* dataset_;
      std::string table_, metadata_, geometry_field_, key_field_;
      mapnik::layer_descriptor desc_;
      bool multiple_geometries_;
      bool use_spatial_index_;
};


#endif // SQLITE_DATASOURCE_HPP
