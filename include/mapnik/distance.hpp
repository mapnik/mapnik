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

#ifndef DISTANCE_HPP
#define DISTANCE_HPP

#include <mapnik/coord.hpp>

namespace mapnik
{
    struct ellipsoid;
    
    // great-circle distance 
    
    class great_circle_distance
    {
    public:
        double operator() (coord2d const& pt0, coord2d const& pt1) const;
    };
    
    // vincenty distance 
    /*
    class vincenty_distance : boost::noncopyble
    {
    public:
        vincenty_distance(ellipsoid const& e);
        double operator() (coord2d const& pt0, coord2d const& pt1) const;
    private:
        ellipsoid & e_;
    };
    */
}

#endif // GEO_UTILS_HPP
