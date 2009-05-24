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

// mapnik
#include <mapnik/proj_transform.hpp>
#include <mapnik/utils.hpp>
// proj4
#include <proj_api.h>

namespace mapnik {
    
    proj_transform::proj_transform(projection const& source, 
                                   projection const& dest)
            : source_(source),
              dest_(dest) 
    {
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(projection::mutex_);
#endif
        is_source_latlong_ = pj_is_latlong(source_.proj_) ? true : false;
        is_dest_latlong_ = pj_is_latlong(dest_.proj_) ? true : false ;
        is_source_equal_dest = (source_ == dest_);
    }
    
    bool proj_transform::forward (double & x, double & y , double & z) const
    {
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(projection::mutex_);
#endif
        if (is_source_equal_dest)
            return true;

        if (is_source_latlong_)
        {
            x *= DEG_TO_RAD;
            y *= DEG_TO_RAD;
        }
        
        if (pj_transform( source_.proj_, dest_.proj_, 1, 
                          0, &x,&y,&z) != 0)
        {
            return false;
        }
        
        if (is_dest_latlong_)
        {
            x *= RAD_TO_DEG;
            y *= RAD_TO_DEG;
        }
        
        return true;
    } 
        
    bool proj_transform::backward (double & x, double & y , double & z) const
    {
#ifdef MAPNIK_THREADSAFE
        mutex::scoped_lock lock(projection::mutex_);
#endif
        if (is_source_equal_dest)
            return true;
      
        if (is_dest_latlong_)
        {
            x *= DEG_TO_RAD;
            y *= DEG_TO_RAD;
        }
        
        if (pj_transform( dest_.proj_, source_.proj_, 1, 
                          0, &x,&y,&z) != 0)
        {
            return false;
        }
        
        if (is_source_latlong_)
        {
            x *= RAD_TO_DEG;
            y *= RAD_TO_DEG;
        }
        
        return true;
    }

    mapnik::projection const& proj_transform::source() const
    {
        return source_;
    }
    mapnik::projection const& proj_transform::dest() const
    {
        return dest_;
    }
 
}
