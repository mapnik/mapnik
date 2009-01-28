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

#ifndef OGR_DATASOURCE_HPP
#define OGR_DATASOURCE_HPP

#include <mapnik/datasource.hpp>
#include <boost/shared_ptr.hpp>
#include <ogrsf_frmts.h>

using mapnik::datasource;
using mapnik::parameters;
using mapnik::query;
using mapnik::coord2d;
using mapnik::featureset_ptr;
using mapnik::layer_descriptor;
using mapnik::Envelope;


class ogr_datasource : public datasource 
{
   public:
      ogr_datasource(parameters const& params);
      virtual ~ogr_datasource ();
      int type() const;
      static std::string name();
      featureset_ptr features(query const& q) const;
      featureset_ptr features_at_point(coord2d const& pt) const;
      Envelope<double> envelope() const;
      layer_descriptor get_descriptor() const;
   private:
      static const std::string name_;
      Envelope<double> extent_;
      OGRDataSource* dataset_;
      OGRLayer* layer_;
      layer_descriptor desc_;
};


#endif // OGR_DATASOURCE_HPP
