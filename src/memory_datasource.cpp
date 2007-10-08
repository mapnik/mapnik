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
#include <mapnik/memory_datasource.hpp>

#include <mapnik/memory_featureset.hpp>
#include <algorithm>

namespace mapnik {
    
    struct accumulate_extent
    {
        accumulate_extent(Envelope<double> & ext)
            : ext_(ext),first_(true) {}
        
        void operator() (feature_ptr feat)
        {
           for (unsigned i=0;i<feat->num_geometries();++i)
           {
              geometry2d & geom = feat->get_geometry(i);
              if ( first_ ) 
              {
                 first_ = false;
                 ext_ = geom.envelope();
              }
              else
              {
                 ext_.expand_to_include(geom.envelope());
              }
           }
        }
        
        Envelope<double> & ext_;
        bool first_;
    };
    
    memory_datasource::memory_datasource()
        : datasource(parameters()) {}
    memory_datasource::~memory_datasource() {}
    
    void memory_datasource::push(feature_ptr feature)
    {
        features_.push_back(feature);
    }
    
    int memory_datasource::type() const
    {
        return datasource::Vector;
    }
    
    featureset_ptr memory_datasource::features(const query& q) const
    {
        return featureset_ptr(new memory_featureset(q.get_bbox(),*this));
    }


    featureset_ptr memory_datasource::features_at_point(coord2d const& pt) const
    {
        return featureset_ptr();
    }
    
    Envelope<double> memory_datasource::envelope() const
    {
        Envelope<double> ext;
        accumulate_extent func(ext);
        std::for_each(features_.begin(),features_.end(),func);
        return ext;      
    }
    
    layer_descriptor memory_datasource::get_descriptor() const
    {
        return layer_descriptor("in-memory datasource","utf-8");
    }
    
    size_t memory_datasource::size() const
    {
        return features_.size();
    }
}
