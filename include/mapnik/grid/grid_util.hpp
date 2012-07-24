/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef GRID_UTIL_HPP
#define GRID_UTIL_HPP

// mapnik
#include <mapnik/grid/grid.hpp>

namespace mapnik {

/*
 * Nearest neighbor resampling for grids
 */

static inline void scale_grid(mapnik::grid::data_type & target,
                              const mapnik::grid::data_type & source,
                              double x_off_f, double y_off_f)
{

    int source_width=source.width();
    int source_height=source.height();

    int target_width=target.width();
    int target_height=target.height();

    if (source_width<1 || source_height<1 ||
        target_width<1 || target_height<1) return;
    int x = 0;
    int y = 0;
    int xs = 0;
    int ys = 0;
    int tw2 = target_width/2;
    int th2 = target_height/2;
    int offs_x = rint((source_width-target_width-x_off_f*2*source_width)/2);
    int offs_y = rint((source_height-target_height-y_off_f*2*source_height)/2);
    unsigned yprt = 0;
    unsigned yprt1 = 0;
    unsigned xprt = 0;
    unsigned xprt1 = 0;

    //no scaling or subpixel offset
    if (target_height == source_height && target_width == source_width && offs_x == 0 && offs_y == 0){
        for (y=0;y<target_height;++y)
            target.setRow(y,source.getRow(y),target_width);
        return;
    }

    for (y=0;y<target_height;++y)
    {
        ys = (y*source_height+offs_y)/target_height;
        int ys1 = ys+1;
        if (ys1>=source_height)
            ys1--;
        if (ys<0)
            ys=ys1=0;
        if (source_height/2<target_height)
            yprt = (y*source_height+offs_y)%target_height;
        else
            yprt = th2;
        yprt1 = target_height-yprt;
        for (x=0;x<target_width;++x)
        {
            xs = (x*source_width+offs_x)/target_width;
            if (source_width/2<target_width)
                xprt = (x*source_width+offs_x)%target_width;
            else
                xprt = tw2;
            xprt1 = target_width-xprt;
            int xs1 = xs+1;
            if (xs1>=source_width)
                xs1--;
            if (xs<0)
                xs=xs1=0;

            mapnik::grid::value_type a = source(xs,ys);
            mapnik::grid::value_type b = source(xs1,ys);
            mapnik::grid::value_type c = source(xs,ys1);
            mapnik::grid::value_type d = source(xs1,ys1);

            if ((a > 0) && (b > 0))
                target(x,y) = b;
            else if ((c > 0) && (d > 0))
                target(x,y) = d;
            else
                target(x,y) = a;
        }
    }
}

}

#endif // GRID_UTIL_HPP
