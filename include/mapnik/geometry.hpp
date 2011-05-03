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

//$Id: geometry.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

// mapnik
#include <mapnik/vertex_vector.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/geom_util.hpp>
// boost
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace mapnik {

enum eGeomType {
    Point = 1,
    LineString,
    Polygon,
    MultiPoint,
    MultiLineString,
    MultiPolygon
};


template <typename T, template <typename> class Container=vertex_vector>
class geometry
{
public:
    typedef T vertex_type;
    typedef typename vertex_type::type value_type;
    typedef Container<vertex_type> container_type;   
private:
    container_type cont_;
    eGeomType type_;
    mutable unsigned itr_;
public:
    
    geometry(eGeomType type)
        : type_(type),
          itr_(0)
    {}
    
    eGeomType type() const 
    {
        return type_;
    }
    
    double area() const 
    {
        double sum = 0.0;
        double x(0);
        double y(0);
        rewind(0);
        double xs = x;
        double ys = y;
        for (unsigned i=0;i<num_points();++i)
        {
            double x0(0);
            double y0(0);
            vertex(&x0,&y0);
            sum += x * y0 - y * x0;
            x = x0;
            y = y0;
        }
        return (sum + x * ys - y * xs) * 0.5;
    }
    
    box2d<double> envelope() const
    {
        box2d<double> result;
        double x(0);
        double y(0);
        rewind(0);
        for (unsigned i=0;i<num_points();++i)
        {
            vertex(&x,&y);
            if (i==0)
            {
                result.init(x,y,x,y);
            }
            else
            {
                result.expand_to_include(x,y);
            }
        }
        return result;
    }

    void label_interior_position(double *x, double *y) const
    {
        // start with the default label position
        label_position(x,y);
        unsigned size = cont_.size();
        // if we are not a polygon, or the default is within the polygon we are done
        if (size < 3 || hit_test(*x,*y,0))
            return;

        // otherwise we find a horizontal line across the polygon and then return the
        // center of the widest intersection between the polygon and the line.

        std::vector<double> intersections; // only need to store the X as we know the y

        double x0=0;
        double y0=0;
        rewind(0);
        unsigned command = vertex(&x0, &y0);
        double x1,y1;
        while (SEG_END != (command=vertex(&x1, &y1)))
        {
            if (command != SEG_MOVETO)
            {
                // if the segments overlap
                if (y0==y1)
                {
                    if (y0==*y)
                    {
                        double xi = (x0+x1)/2.0;
                        intersections.push_back(xi);
                    }
                }
                // if the path segment crosses the bisector
                else if ((y0 <= *y && y1 >= *y) ||
                        (y0 >= *y && y1 <= *y))
                {
                    // then calculate the intersection
                    double xi = x0;
                    if (x0 != x1)
                    {
                        double m = (y1-y0)/(x1-x0);
                        double c = y0 - m*x0;
                        xi = (*y-c)/m;
                    }

                    intersections.push_back(xi);
                }
            }
            x0 = x1;
            y0 = y1;
        }
        // no intersections we just return the default
        if (intersections.empty())
            return;
        x0=intersections[0];
        double max_width = 0;
        for (unsigned ii = 1; ii < intersections.size(); ++ii)
        {
            double x1=intersections[ii];
            double xc=(x0+x1)/2.0;
            double width = fabs(x1-x0);
            if (width > max_width && hit_test(xc,*y,0))
            {
                *x=xc;
                max_width = width;
            }
        }
    }

    /* center of gravity centroid
      - best visually but does not work with multipolygons
    */
    void label_position(double *x, double *y) const
    {
        if (type_ == LineString || type_ == MultiLineString)
        {
            middle_point(x,y);
            return;
        }

        unsigned size = cont_.size();
        if (size < 3) 
        {
            cont_.get_vertex(0,x,y);
            return;
        }
           
        double ai;
        double atmp = 0;
        double xtmp = 0;
        double ytmp = 0;
        double x0 =0;
        double y0 =0;
        double x1 =0;
        double y1 =0;
        double ox =0;
        double oy =0;
           
        unsigned i;
           
        // Use first point as origin to improve numerical accuracy
        cont_.get_vertex(0,&ox,&oy);

        for (i = 0; i < size-1; i++)
        {
            cont_.get_vertex(i,&x0,&y0);
            cont_.get_vertex(i+1,&x1,&y1);
               
            x0 -= ox; y0 -= oy;
            x1 -= ox; y1 -= oy;

            ai = x0 * y1 - x1 * y0;
            atmp += ai;
            xtmp += (x1 + x0) * ai;
            ytmp += (y1 + y0) * ai;
        }    
        if (atmp != 0)
        {
            *x = (xtmp/(3*atmp)) + ox;
            *y = (ytmp/(3*atmp)) + oy;
            return;
        }
        *x=x0;
        *y=y0;            
    }

    /* center of bounding box centroid */
    void label_position2(double *x, double *y) const
    {

        box2d<double> box = envelope();
        *x = box.center().x;
        *y = box.center().y; 
    }

    /* summarized distance centroid */
    void label_position3(double *x, double *y) const
    {
        if (type_ == LineString || type_ == MultiLineString)
        {
            middle_point(x,y);
            return;
        }

        unsigned i = 0;
        double l = 0.0;
        double tl = 0.0;
        double cx = 0.0;
        double cy = 0.0;
        double x0 = 0.0;
        double y0 = 0.0;
        double x1 = 0.0;
        double y1 = 0.0;
        unsigned size = cont_.size();   
        for (i = 0; i < size-1; i++)
        {
            cont_.get_vertex(i,&x0,&y0);
            cont_.get_vertex(i+1,&x1,&y1);
            l = distance(x0,y0,x1,y1);
            cx += l * (x1 + x0)/2;
            cy += l * (y1 + y0)/2;
            tl += l;
        }
        *x = cx / tl;
        *y = cy / tl;
    }
                 
    void middle_point(double *x, double *y) const
    {
        // calculate mid point on path
        double x0=0;
        double y0=0;
        double x1=0;
        double y1=0;
      
        unsigned size = cont_.size();
        if (size == 1)
        {
            cont_.get_vertex(0,x,y); 
        }
        else if (size == 2)
        {
            cont_.get_vertex(0,&x0,&y0);
            cont_.get_vertex(1,&x1,&y1);
            *x = 0.5 * (x1 + x0);
            *y = 0.5 * (y1 + y0);    
        }
        else
        {
            double len=0.0;
            for (unsigned pos = 1; pos < size; ++pos)
            {
                cont_.get_vertex(pos-1,&x0,&y0);
                cont_.get_vertex(pos,&x1,&y1);
                double dx = x1 - x0;
                double dy = y1 - y0;
                len += std::sqrt(dx * dx + dy * dy);
            }
            double midlen = 0.5 * len;
            double dist = 0.0;
            for (unsigned pos = 1; pos < size;++pos)
            {
                cont_.get_vertex(pos-1,&x0,&y0);
                cont_.get_vertex(pos,&x1,&y1);
                double dx = x1 - x0;
                double dy = y1 - y0; 
                double seg_len = std::sqrt(dx * dx + dy * dy);
                if (( dist + seg_len) >= midlen)
                {
                    double r = (midlen - dist)/seg_len;
                    *x = x0 + (x1 - x0) * r;
                    *y = y0 + (y1 - y0) * r;
                    break;
                }
                dist += seg_len;
            }
        }  
    }
    
    void push_vertex(value_type x, value_type y, CommandType c) 
    {
        cont_.push_back(x,y,c);
    }

    void line_to(value_type x,value_type y)
    {
        push_vertex(x,y,SEG_LINETO);
    }
         
    void move_to(value_type x,value_type y)
    {
        push_vertex(x,y,SEG_MOVETO);
    }
    
    unsigned num_points() const
    {
        return cont_.size();
    }
         
    unsigned vertex(double* x, double* y) const
    {
        return cont_.get_vertex(itr_++,x,y);
    }         

    unsigned get_vertex(unsigned pos, double* x, double* y) const
    {
        return cont_.get_vertex(pos, x, y);
    }         

    void rewind(unsigned ) const
    {
        itr_=0;
    }
         
    bool hit_test(value_type x, value_type y, double tol) const
    {      
        if (cont_.size() == 1) {
            // Handle points
            double x0, y0;
            cont_.get_vertex(0, &x0, &y0);
            return distance(x, y, x0, y0) <= fabs(tol);
        } else if (cont_.size() > 1) {
            bool inside=false;
            double x0=0;
            double y0=0;
            rewind(0);
            vertex(&x0, &y0);
                
            unsigned command;
            double x1,y1;
            while (SEG_END != (command=vertex(&x1, &y1)))
            {
                if (command == SEG_MOVETO)
                {
                    x0 = x1;
                    y0 = y1;
                    continue;
                }               
                if ((((y1 <= y) && (y < y0)) ||
                     ((y0 <= y) && (y < y1))) &&
                    ( x < (x0 - x1) * (y - y1)/ (y0 - y1) + x1))
                    inside=!inside;
                x0=x1;
                y0=y1;
            }
            return inside;
        }
        return false;
    } 
         
    void set_capacity(size_t size) 
    {
        cont_.set_capacity(size);
    }
};
   
typedef geometry<vertex2d,vertex_vector> geometry_type; 
typedef boost::shared_ptr<geometry_type> geometry_ptr;
typedef boost::ptr_vector<geometry_type> geometry_containter;

}

#endif //GEOMETRY_HPP
