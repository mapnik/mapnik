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

#ifndef GEOM_UTIL_HH
#define GEOM_UTIL_HH

#include "geometry.hh"
#include <cmath>

namespace mapnik
{

    template <typename T>
        bool clip_test(T p,T q,double& tmin,double& tmax)
    {
        double r;
        bool result=true;
        if (p<0.0)
        {
            r=q/p;
            if (r>tmax) result=false;
            else if (r>tmin) tmin=r;
        }
        else if (p>0.0)
        {
            r=q/p;
            if (r<tmin) result=false;
            else if (r<tmax) tmax=r;
        } else if (q<0.0) result=false;
        return result;
    }

    template <typename T,typename Image>
        bool clip_line(T& x0,T& y0,T& x1,T& y1,const Image* image)
    {
        double tmin=0.0;
        double tmax=1.0;
        double dx=x1-x0;
        if (clip_test<double>(-dx,x0,tmin,tmax))
        {
            if (clip_test<double>(dx,image->width()-x0,tmin,tmax))
            {
                double dy=y1-y0;
                if (clip_test<double>(-dy,y0,tmin,tmax))
                {
                    if (clip_test<double>(dy,image->height()-y0,tmin,tmax))
                    {
                        if (tmax<1.0)
                        {
                            x1=static_cast<T>(x0+tmax*dx);
                            y1=static_cast<T>(y0+tmax*dy);
                        }
                        if (tmin>0.0)
                        {
                            x0+=static_cast<T>(tmin*dx);
                            y0+=static_cast<T>(tmin*dy);
                        }
                        return true;
                    }
                }
            }
        }
        return false;
    }

    inline bool point_inside_path(double x,double y,const geometry<vertex2d>& geom)
    {
	bool inside=false;
	if (geom.num_points()>2) 
	{
	    geometry<vertex2d>::path_iterator<NO_SHIFT> itr=geom.begin<NO_SHIFT>();
	    geometry<vertex2d>::path_iterator<NO_SHIFT> end=geom.end<NO_SHIFT>();
	    
	    double x0=itr->x;
	    double y0=itr->y;
	    double x1,y1;
	    while (++itr!=end) 
	    {
		if (itr->cmd == SEG_MOVETO)
		{
		    x0=itr->x;
		    y0=itr->y;
		    continue;
		}		
		x1=itr->x;
		y1=itr->y;
		if ((((y1 <= y) && (y < y0)) ||
		     ((y0 <= y) && (y < y1))) &&
		    ( x < (x0 - x1) * (y - y1)/ (y0 - y1) + x1))
		    inside=!inside;
		x0=itr->x;
		y0=itr->y;
	    }
	}
	return inside;
    }

#define TOL 0.00001

/*
      (Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
  s = -----------------------------
                 L^2
*/

    inline bool point_in_circle(double x,double y,double cx,double cy,double r)
    {
	double dx = x - cx;
	double dy = y - cy;
	double d2 = dx * dx + dy * dy;
	return (d2 <= r * r);
    }
    
    inline bool point_on_segment(double x,double y,double x0,double y0,double x1,double y1)
    {	
	double dx = x1 - x0;
	double dy = y1 - y0;
	if ( fabs(dx) > TOL  ||  fabs(dy) > TOL )
	{
	    double s = (y0 - y) * dx - (x0 - x) * dy;
	    return ( fabs (s) < TOL ) ;
	} 
	return false;
    }

    inline bool point_on_segment2(double x,double y,double x0,double y0,double x1,double y1)
    {	 
	double d  = sqrt ((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
	double d0 = sqrt ((x0 - x) * (x0 - x) + (y0 - y) * (y0 - y));
	double d1 = sqrt ((x1 - x) * (x1 - x) + (y1 - y) * (y1 - y));
	double d2 = d0 + d1;
	return ( d2 - d < 0.01);
    }
    
#undef TOL

    inline bool point_on_path(double x,double y,const geometry<vertex2d>& geom)
    {
	bool on_path=false;
	if (geom.num_points()>1) 
	{
	    geometry<vertex2d>::path_iterator<NO_SHIFT> itr=geom.begin<NO_SHIFT>();
	    geometry<vertex2d>::path_iterator<NO_SHIFT> end=geom.end<NO_SHIFT>();
	    
	    double x0=itr->x;
	    double y0=itr->y;
	    while (++itr!=end) 
	    {
		if (itr->cmd == SEG_MOVETO)
		{
		    x0=itr->x;
		    y0=itr->y;
		    continue;
		}		
		double x1=itr->x;
		double y1=itr->y;
		
		on_path = point_on_segment(x,y,x0,y0,x1,y1);
		if (on_path)
		    break;
		
		x0=itr->x;
		y0=itr->y;
	    }
	}
	return on_path;
    }

    inline bool point_on_points (double x,double y,const geometry_type& geom) 
    {
	geometry<vertex2d>::path_iterator<NO_SHIFT> itr=geom.begin<NO_SHIFT>();
	geometry<vertex2d>::path_iterator<NO_SHIFT> end=geom.end<NO_SHIFT>();
	while (itr!=end) 
	{
	    double dx = x - itr->x;
	    double dy = y - itr->y;
	    double d = sqrt(dx*dx+dy*dy);
	    
	    if (d < 0.02)
	    {
		std::cout<<"d="<<d<<" x="<<x<<" y="<<y<<" itr->x="<<itr->x<<" itr->y="<<itr->y<<std::endl;
		return true;
	    }
	    ++itr;
	}
	return false; 
    }
}

#endif                                            //GEOM_UTIL_HH
