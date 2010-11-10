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

#ifndef GEOS_FEATURE_PTR_HPP
#define GEOS_FEATURE_PTR_HPP

// geos
#include <geos_c.h>
  
class geos_feature_ptr
{
public:
    explicit geos_feature_ptr (GEOSGeometry* const feat)
        : feat_ (feat)
    {
    }
    
    ~geos_feature_ptr ()
    {
        if (feat_ != NULL)
            GEOSGeom_destroy(feat_);
    }

    GEOSGeometry* operator*()
    {
        return feat_;
    }

private:
    GEOSGeometry* feat_;
};

#endif // GEOS_FEATURE_PTR_HPP

