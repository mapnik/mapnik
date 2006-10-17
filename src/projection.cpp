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

// proj4
#include <proj_api.h>
// mapnik
#include <mapnik/projection.hpp>

namespace mapnik {
    projection::projection(std::string  params)
        : params_(params)
    { 
        init(); //
    }
    
    projection::projection(projection const& rhs)
        : params_(rhs.params_) 
    {
        init(); //
    }
        
    projection& projection::operator=(projection const& rhs) 
    { 
        projection tmp(rhs);
        swap(tmp);
        return *this;
    }
        
    bool projection::is_initialized() const
    {
        return proj_ ? true : false;
    }
    
    bool projection::is_geographic() const
    {
        return pj_is_latlong(proj_);  
    }
    
    std::string const& projection::params() const
    {
        return params_;
    }
    
    void projection::forward(double & x, double &y ) const
    {
        projUV p;
        p.u = x * DEG_TO_RAD;
        p.v = y * DEG_TO_RAD;
        p = pj_fwd(p,proj_);
        x = p.u;
        y = p.v;
    }
    
    void projection::inverse(double & x,double & y) const
    {
        projUV p;
        p.u = x;
        p.v = y;
        p = pj_inv(p,proj_);
        x = RAD_TO_DEG * p.u;
        y = RAD_TO_DEG * p.v;
    }
    
    projection::~projection() 
    {
        if (proj_) pj_free(proj_);
    }
    
    void projection::init()
    {
        proj_=pj_init_plus(params_.c_str());
        if (!proj_) throw proj_init_error(params_);
    }
    
    void projection::swap (projection& rhs)
    {
        std::swap(params_,rhs.params_);
        init ();
    }
}
