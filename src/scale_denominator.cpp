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
#include <cmath>
#include <mapnik/map.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/distance.hpp>
#include <mapnik/scale_denominator.hpp>

namespace mapnik {
 
    static const double dpi = 90.0; // display resolution
    
    double scale_denominator(Map const& map,projection const& prj)
    {
        double map_width = map.getWidth();
        double map_height = map.getHeight();
        Envelope<double> const& extent = map.getCurrentExtent();
        double x0 = extent.minx();
        double y0 = extent.miny();
        double x1 = extent.maxx();
        double y1 = extent.maxy();
        
        if (!prj.is_geographic())
        {
            prj.inverse(x0,y0);
            prj.inverse(x1,y1); 
        }
        great_circle_distance distance;
        double d1 = distance(coord2d(x0,y0),coord2d(x1, y1));
        double d0 = sqrt(map_width * map_width + map_height * map_height) / dpi * 0.0254;
        return d1 / d0;
    }
}
