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

#ifndef KISMET_FEATURESET_HPP
#define KISMET_FEATURESET_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/unicode.hpp> 
#include <mapnik/wkb.hpp> 

// boost
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

//STL
#include <list>

#include "kismet_types.hpp"
  
  
class kismet_featureset : public mapnik::Featureset
{
   public:
      kismet_featureset(const std::list<kismet_network_data> &knd_list,
                        std::string const& encoding);
      virtual ~kismet_featureset();
      mapnik::feature_ptr next();
    
   private:
      const std::list<kismet_network_data> &knd_list_;
      boost::scoped_ptr<mapnik::transcoder> tr_;
      mapnik::wkbFormat format_;
      bool multiple_geometries_;
      int feature_id;
      std::list<kismet_network_data>::const_iterator knd_list_it;
      mapnik::projection source_;
};

#endif // KISMET_FEATURESET_HPP
