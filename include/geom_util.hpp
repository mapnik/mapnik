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

//$Id: geom_util.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef GEOM_UTIL_HPP
#define GEOM_UTIL_HPP

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
    bool clip_line(T& x0,T& y0,T& x1,T& y1,Envelope<T> const& box)
    {
        double tmin=0.0;
        double tmax=1.0;
        double dx=x1-x0;
        if (clip_test<double>(-dx,x0,tmin,tmax))
        {
            if (clip_test<double>(dx,box.width()-x0,tmin,tmax))
            {
                double dy=y1-y0;
                if (clip_test<double>(-dy,y0,tmin,tmax))
                {
                    if (clip_test<double>(dy,box.height()-y0,tmin,tmax))
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
    
    template <typename Iter> 
    inline bool point_inside_path(double x,double y,Iter start,Iter end)
    {
        bool inside=false;
        double x0=boost::get<0>(*start);
        double y0=boost::get<1>(*start);
        
        double x1,y1;
        while (++start!=end) 
        {
            if ( boost::get<2>(*start) == SEG_MOVETO)
            {
                x0 = boost::get<0>(*start);
                y0 = boost::get<1>(*start);
                continue;
            }		
            x1=boost::get<0>(*start);
            y1=boost::get<1>(*start);
            
            if ((((y1 <= y) && (y < y0)) ||
                 ((y0 <= y) && (y < y1))) &&
                ( x < (x0 - x1) * (y - y1)/ (y0 - y1) + x1))
                inside=!inside;
            x0=x1;
            y0=y1;
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
    template <typename Iter> 
    inline bool point_on_path(double x,double y,Iter start,Iter end)
    {
        return false;
    }
    
    template <typename Iter> 
    inline bool point_on_points (double x,double y,Iter start,Iter end) 
    {
        return false; 
    }

    struct filter_in_box
    {
        Envelope<double> box_;
        explicit filter_in_box(const Envelope<double>& box)
            : box_(box) {}

        bool pass(const Envelope<double>& extent) const
        {
            return extent.intersects(box_);
        }
    };

    struct filter_at_point
    {
        coord2d pt_;
        explicit filter_at_point(const coord2d& pt)
            : pt_(pt) {}
        bool pass(const Envelope<double>& extent) const
        {
            return extent.contains(pt_);
        }
    };
}

#endif                                            //GEOM_UTIL_HPP
