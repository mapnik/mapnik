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

#ifndef KISMET_DATASOURCE_HPP
#define KISMET_DATASOURCE_HPP

// STL
#include <list>

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp> 

// boost
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

// sqlite
#include "kismet_types.hpp"

class kismet_datasource : public mapnik::datasource 
{
   public:
      kismet_datasource(mapnik::parameters const& params);
      virtual ~kismet_datasource ();
      int type() const;
      static std::string name();
      mapnik::featureset_ptr features(mapnik::query const& q) const;
      mapnik::featureset_ptr features_at_point(mapnik::coord2d const& pt) const;
      mapnik::Envelope<double> envelope() const;
      mapnik::layer_descriptor get_descriptor() const;
    
   private:
      void run (const std::string &host, const unsigned int port);
  
      static const std::string name_;
      mapnik::Envelope<double> extent_;
      mutable bool extent_initialized_;
      int type_;
      mapnik::layer_descriptor desc_;
      boost::shared_ptr<boost::thread> kismet_thread;
};


#endif // KISMET_DATASOURCE_HPP
