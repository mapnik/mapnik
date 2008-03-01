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

// $Id$

#ifndef OSM_DATASOURCE_HPP
#define OSM_DATASOURCE_HPP

#include <mapnik/datasource.hpp>
#include <mapnik/envelope.hpp>

#include "osm.h"


using mapnik::datasource;
using mapnik::parameters;
using mapnik::query;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::coord2d;
using mapnik::Envelope;

class osm_datasource : public datasource
{
   public:
      osm_datasource(const parameters &params);
      virtual ~osm_datasource();
    
	  // these must be overridden
      int type() const;
      featureset_ptr features(const query& q) const;
      featureset_ptr features_at_point(coord2d const& pt) const;
      Envelope<double> envelope() const;
      layer_descriptor get_descriptor() const;   
	  static std::string name() { return name_; }
	  
   private:
      osm_datasource(const osm_datasource&);
      osm_datasource& operator=(const osm_datasource&);
   private:
      Envelope<double> extent_;
	  osm_dataset * osm_data_;
	  int type_;
	  layer_descriptor desc_;
	  static const std::string name_;
};

#endif //OSM_DATASOURCE_HPP
