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

#ifndef MAPNIK_GEOM_UTIL_HPP
#define MAPNIK_GEOM_UTIL_HPP

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/vertex.hpp>

// boost
#include <boost/tuple/tuple.hpp>

// stl
#include <cmath>
#include <vector>

namespace mapnik
{
template <typename T>
bool clip_test(T p,T q,double& tmin,double& tmax)
{
    double r = 0;
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
bool clip_line(T& x0,T& y0,T& x1,T& y1,box2d<T> const& box)
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

    double x1 = 0;
    double y1 = 0;
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

inline bool point_in_circle(double x,double y,double cx,double cy,double r)
{
    double dx = x - cx;
    double dy = y - cy;
    double d2 = dx * dx + dy * dy;
    return (d2 <= r * r);
}

template <typename T>
inline T sqr(T x)
{
    return x * x;
}

inline double distance2(double x0,double y0,double x1,double y1)
{
    double dx = x1 - x0;
    double dy = y1 - y0;
    return sqr(dx) + sqr(dy);
}

inline double distance(double x0,double y0, double x1,double y1)
{
    return std::sqrt(distance2(x0,y0,x1,y1));
}

inline double point_to_segment_distance(double x, double y,
                                        double ax, double ay,
                                        double bx, double by)
{
    double len2 = distance2(ax,ay,bx,by);

    if (len2 < 1e-14)
    {
        return distance(x,y,ax,ay);
    }

    double r = ((x - ax)*(bx - ax) + (y - ay)*(by -ay))/len2;
    if ( r < 0 )
    {
        return distance(x,y,ax,ay);
    }
    else if (r > 1)
    {
        return distance(x,y,bx,by);
    }
    double s = ((ay - y)*(bx - ax) - (ax - x)*(by - ay))/len2;
    return std::fabs(s) * std::sqrt(len2);
}

template <typename Iter>
inline bool point_on_path(double x,double y,Iter start,Iter end, double tol)
{
    double x0=boost::get<0>(*start);
    double y0=boost::get<1>(*start);
    double x1 = 0;
    double y1 = 0;
    while (++start != end)
    {
        if ( boost::get<2>(*start) == SEG_MOVETO)
        {
            x0 = boost::get<0>(*start);
            y0 = boost::get<1>(*start);
            continue;
        }
        x1=boost::get<0>(*start);
        y1=boost::get<1>(*start);

        double distance = point_to_segment_distance(x,y,x0,y0,x1,y1);
        if (distance < tol)
            return true;
        x0=x1;
        y0=y1;
    }
    return false;
}

// filters
struct filter_in_box
{
    box2d<double> box_;
    explicit filter_in_box(const box2d<double>& box)
        : box_(box) {}

    bool pass(const box2d<double>& extent) const
    {
        return extent.intersects(box_);
    }
};

struct filter_at_point
{
    coord2d pt_;
    explicit filter_at_point(const coord2d& pt)
        : pt_(pt) {}
    bool pass(const box2d<double>& extent) const
    {
        return extent.contains(pt_);
    }
};

////////////////////////////////////////////////////////////////////////////
template <typename PathType>
double path_length(PathType & path)
{
    double x0 = 0;
    double y0 = 0;
    double x1 = 0;
    double y1 = 0;
    path.rewind(0);
    unsigned command = path.vertex(&x0,&y0);
    if (command == SEG_END) return 0;
    double length = 0;
    while (SEG_END != (command = path.vertex(&x1, &y1)))
    {
        length += distance(x0,y0,x1,y1);
        x0 = x1;
        y0 = y1;
    }
    return length;
}

namespace label {

template <typename PathType>
bool middle_point(PathType & path, double & x, double & y)
{
    double x0 = 0;
    double y0 = 0;
    double x1 = 0;
    double y1 = 0;
    double mid_length = 0.5 * path_length(path);
    path.rewind(0);
    unsigned command = path.vertex(&x0,&y0);
    if (command == SEG_END) return false;
    double dist = 0.0;
    while (SEG_END != (command = path.vertex(&x1, &y1)))
    {
        double seg_length = distance(x0, y0, x1, y1);

        if ( dist + seg_length >= mid_length)
        {
            double r = (mid_length - dist)/seg_length;
            x = x0 + (x1 - x0) * r;
            y = y0 + (y1 - y0) * r;
            break;
        }
        dist += seg_length;
        x0 = x1;
        y0 = y1;
    }
    return true;
}

template <typename PathType>
bool centroid(PathType & path, double & x, double & y)
{
    double x0 = 0.0;
    double y0 = 0.0;
    double x1 = 0.0;
    double y1 = 0.0;
    double start_x;
    double start_y;

    path.rewind(0);
    unsigned command = path.vertex(&x0, &y0);
    if (command == SEG_END) return false;

    start_x = x0;
    start_y = y0;

    double atmp = 0.0;
    double xtmp = 0.0;
    double ytmp = 0.0;
    unsigned count = 1;
    while (SEG_END != (command = path.vertex(&x1, &y1)))
    {
        double dx0 = x0 - start_x;
        double dy0 = y0 - start_y;
        double dx1 = x1 - start_x;
        double dy1 = y1 - start_y;

        double ai = dx0 * dy1 - dx1 * dy0;
        atmp += ai;
        xtmp += (dx1 + dx0) * ai;
        ytmp += (dy1 + dy0) * ai;
        x0 = x1;
        y0 = y1;
        ++count;
    }

    if (count <= 2) {
        x = (start_x + x0) * 0.5;
        y = (start_y + y0) * 0.5;
        return true;
    }

    if (atmp != 0)
    {
        x = (xtmp/(3*atmp)) + start_x;
        y = (ytmp/(3*atmp)) + start_y;
    }
    else
    {
        x = x0;
        y = y0;
    }
    return true;
}

template <typename PathType>
bool hit_test(PathType & path, double x, double y, double tol)
{
    bool inside=false;
    double x0 = 0;
    double y0 = 0;
    double x1 = 0;
    double y1 = 0;
    path.rewind(0);
    unsigned command = path.vertex(&x0, &y0);
    if (command == SEG_END) return false;
    unsigned count = 0;
    while (SEG_END != (command = path.vertex(&x1, &y1)))
    {
        ++count;
        if (command == SEG_MOVETO)
        {
            x0 = x1;
            y0 = y1;
            continue;
        }
        if ((((y1 <= y) && (y < y0)) ||
             ((y0 <= y) && (y < y1))) &&
            (x < (x0 - x1) * (y - y1)/ (y0 - y1) + x1))
            inside=!inside;

        x0 = x1;
        y0 = y1;
    }

    if (count == 0) // one vertex
    {
        return distance(x, y, x0, y0) <= fabs(tol);
    }
    return inside;
}

template <typename PathType>
bool interior_position(PathType & path, double & x, double & y)
{
    // start with the centroid
    if (!label::centroid(path, x,y))
        return false;

    // if we are not a polygon, or the default is within the polygon we are done
    if (hit_test(path,x,y,0.001))
        return true;

    // otherwise we find a horizontal line across the polygon and then return the
    // center of the widest intersection between the polygon and the line.

    std::vector<double> intersections; // only need to store the X as we know the y

    double x0 = 0;
    double y0 = 0;
    path.rewind(0);
    unsigned command = path.vertex(&x0, &y0);
    double x1 = 0;
    double y1 = 0;
    while (SEG_END != (command = path.vertex(&x1, &y1)))
    {
        if (command != SEG_MOVETO)
        {
            // if the segments overlap
            if (y0==y1)
            {
                if (y0==y)
                {
                    double xi = (x0+x1)/2.0;
                    intersections.push_back(xi);
                }
            }
            // if the path segment crosses the bisector
            else if ((y0 <= y && y1 >= y) ||
                     (y0 >= y && y1 <= y))
            {
                // then calculate the intersection
                double xi = x0;
                if (x0 != x1)
                {
                    double m = (y1-y0)/(x1-x0);
                    double c = y0 - m*x0;
                    xi = (y-c)/m;
                }

                intersections.push_back(xi);
            }
        }
        x0 = x1;
        y0 = y1;
    }
    // no intersections we just return the default
    if (intersections.empty())
        return true;
    x0=intersections[0];
    double max_width = 0;
    for (unsigned ii = 1; ii < intersections.size(); ++ii)
    {
        double x1=intersections[ii];
        double xc=(x0+x1)/2.0;
        double width = std::fabs(x1-x0);
        if (width > max_width && hit_test(path,xc,y,0))
        {
            x=xc;
            max_width = width;
            break;
        }
    }
    return true;
}

}}

#endif // MAPNIK_GEOM_UTIL_HPP
