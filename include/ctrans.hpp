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

//$Id: ctrans.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef CTRANS_HPP
#define CTRANS_HPP

#include "envelope.hpp"
#include "coord_array.hpp"

namespace mapnik
{
    typedef coord_array<coord2d> CoordinateArray;
    
    template <typename Transform,typename Geometry>
    struct MAPNIK_DECL coord_transform
    {
	coord_transform(Transform const& t,Geometry& geom)
	    : t_(t), geom_(geom) {}
	
	unsigned  vertex(double *x , double *y) const
	{
	    unsigned command = geom_.vertex(x,y);
	    *x = t_.forward_x_(x);
	    *y = t_.forward_y_(y);
	    return command;
	}
	void rewind (unsigned pos)
	{
	    geom_.rewind(pos);
	}
	
    private:
	Transform const& t_;
	Geometry& geom_;
    };
    
    class CoordTransform
    {
    private:
	    int width;
	    int height;
	    double scale_;
	    Envelope<double> extent;
    public:
	CoordTransform(int width,int height,const Envelope<double>& extent)
            :width(width),height(height),extent(extent)
	{
	    double sx=((double)width)/extent.width();
	    double sy=((double)height)/extent.height();
	    scale_=std::min(sx,sy);
	}
	
	inline double scale() const
	{
	    return scale_;
	}
	
	inline void forward_x(double* x) const 
	{
	    *x = (*x - extent.minx() ) * scale_;   
	}
	
	inline void forward_y(double* y) const 
	{
	    *y = (extent.maxy() - *y) * scale_;
	}

	inline double forward_x_(double* x) const 
	{
	    return (*x - extent.minx() ) * scale_;   
	}
	
	inline double forward_y_(double* y) const 
	{
	    return (extent.maxy() - *y) * scale_;
	}

	inline void backward_x(double* x) const
	{
	    *x = extent.minx() + *x/scale_;
	}
	
	inline void backward_y(double* y) const
	{
	    *y = extent.maxy() - *y/scale_;
	}
	
	inline coord2d& forward(coord2d& c) const
	{
	    forward_x(&c.x);
	    forward_y(&c.y);
	    return c;
	}

	inline coord2d& backward(coord2d& c) const
	{
	    backward_x(&c.x);
	    backward_y(&c.y);
	    return c;
	}

	inline Envelope<double> forward(const Envelope<double>& e) const
	{
	    double x0 = e.minx();
	    double y0 = e.miny();
	    double x1 = e.maxx();
	    double y1 = e.maxy();
	    forward_x(&x0);
	    forward_y(&y0);
	    forward_x(&x1);
	    forward_y(&y1);
	    return Envelope<double>(x0,y0,x1,y1);
	}

	inline Envelope<double> backward(const Envelope<double>& e) const
	{
	    double x0 = e.minx();
	    double y0 = e.miny();
	    double x1 = e.maxx();
	    double y1 = e.maxy();
	    backward_x(&x0);
	    backward_y(&y0);
	    backward_x(&x1);
	    backward_y(&y1);
	    
	    return Envelope<double>(x0,y0,x1,y1);
	}

	inline CoordinateArray& forward(CoordinateArray& coords) const
	{
	    for (unsigned i=0;i<coords.size();++i)
	    {
		forward(coords[i]);
	    }
	    return coords;
	}
	
	inline CoordinateArray& backward(CoordinateArray& coords) const
	{
	    for (unsigned i=0;i<coords.size();++i)
	    {
		backward(coords[i]);
	    }
	    return coords;
	}
    };
}
#endif                                            //CTRANS_HPP
