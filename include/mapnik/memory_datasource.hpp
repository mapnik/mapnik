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

//$Id$

#ifndef MEMORY_DATASOURCE_HPP
#define MEMORY_DATASOURCE_HPP

#include <mapnik/datasource.hpp>
#include <mapnik/feature_factory.hpp> // TODO remove
#include <vector>

namespace mapnik {
    
   class MAPNIK_DECL memory_datasource : public datasource
   {
      friend class memory_featureset;
   public:
      memory_datasource();
      virtual ~memory_datasource();
      void push(feature_ptr feature);
      int type() const;
      featureset_ptr features(const query& q) const;
      featureset_ptr features_at_point(coord2d const& pt) const;
      Envelope<double> envelope() const;
      layer_descriptor get_descriptor() const;
      size_t size() const;
   private:
      std::vector<mapnik::feature_ptr> features_;
   }; 
   
   // This class implements a simple way of displaying point-based data
   // TODO -- possible redesign, move into separate file
   //
   
   class MAPNIK_DECL point_datasource : public mapnik::memory_datasource {
   public:
      point_datasource() : feat_id(0) {}
      void add_point(double x, double y, const char* key, const char* value) {
         mapnik::feature_ptr feature(mapnik::feature_factory::create(feat_id++));
         mapnik::geometry2d * pt = new mapnik::point_impl;
         pt->move_to(x,y);
         feature->add_geometry(pt);
         mapnik::transcoder tr("utf-8");
         (*feature)[key] = tr.transcode(value);
         this->push(feature);
      }
      
      int type() const { return mapnik::datasource::Vector; }
      
   private:
      int feat_id;
   };   
}

#endif // MEMORY_DATASOURCE_HPP
