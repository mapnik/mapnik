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

#include <proj_api.h>

#include <mapnik/proj_transform.hpp>

namespace mapnik {
    
    proj_transform::proj_transform(projection const& source, 
                                   projection const& dest)
            : source_(source),
              dest_(dest) 
    {
        is_source_latlong_ = pj_is_latlong(source_.proj_);
        is_dest_latlong_ = pj_is_latlong(dest_.proj_);
    }
    
    bool proj_transform::forward (double & x, double & y , double & z) const
    {
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
}
