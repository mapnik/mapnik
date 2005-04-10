/* This file is part of Mapnik (c++ mapping toolkit)
 * Copyright (C) 2005 Artem Pavlenko
 *
 * Mapnik is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

//$Id$

#ifndef CTRANS_HH
#define CTRANS_HH

#include "envelope.hh"
#include "coord_array.hh"

namespace mapnik
{
    typedef coord_array<coord2d> CoordinateArray;

    class CoordTransform
    {
    private:
	int width;
	int height;
	double scale_;
	Envelope<double> extent;
	coord2d center_;
    public:
	CoordTransform(int width,int height,const Envelope<double>& extent)
            :width(width),height(height),extent(extent),center_(extent.center())
	{
	    double sx=((double)width)/extent.width();
	    double sy=((double)height)/extent.height();
	    scale_=std::min(sx,sy);
	}
	
	inline double scale() const
	{
	    return scale_;
	}
	
	double forward_x(double x) const 
	{
	    return (x-center_.x)*scale_+0.5*width;
	}
	
	double forward_y(double y) const 
	{
	    return -(y-center_.y)*scale_+0.5*height;
	}

	inline coord2d& forward(coord2d& c) const
	{
	    c.x=(c.x-center_.x)*scale_+0.5*width;
	    c.y=-(c.y-center_.y)*scale_+0.5*height;
	    return c;
	}

	inline coord2d& backward(coord2d& c) const
	{
	    c.x=(c.x-0.5*width)/scale_+center_.x;
	    c.y=-(c.y-0.5*height)/scale_+center_.y;
	    return c;
	}

	inline Envelope<double> forward(const Envelope<double>& e) const
	{
	    
	    double x0=(e.minx()-center_.x)*scale_+0.5*width;
	    double x1=(e.maxx()-center_.x)*scale_+0.5*width;
	    
	    double y0=-(e.miny()-center_.y)*scale_+0.5*height;
	    double y1=-(e.maxy()-center_.y)*scale_+0.5*height;
	    
	    return Envelope<double>(x0,y0,x1,y1);
	}

	inline Envelope<double> backward(const Envelope<double>& e) const
	{

	    double x0=(e.minx()-0.5*width)/scale_+center_.x;
	    double x1=(e.maxx()-0.5*width)/scale_+center_.x;

	    double y0=-(e.miny()-0.5*height)/scale_+center_.y;
	    double y1=-(e.maxy()-0.5*height)/scale_+center_.y;

	    return Envelope<double>(x0,y0,x1,y1);
	}

	inline CoordinateArray& forward(CoordinateArray& coords) const
	{
	    for (unsigned i=0;i<coords.size();++i)
	    {
		coords[i].x=(coords[i].x-center_.x)*scale_+0.5*width;
		coords[i].y=-(coords[i].y-center_.y)*scale_+0.5*height;
	    }
	    return coords;
	}
	
	inline CoordinateArray& backward(CoordinateArray& coords) const
	{
	    for (unsigned i=0;i<coords.size();++i)
	    {
		coords[i].x=(coords[i].x-0.5*width)/scale_+center_.x;
		coords[i].y=-(coords[i].y-0.5*height)/scale_+center_.y;
	    }
	    return coords;
	}
    };
}
#endif                                            //CTRANS_HH
