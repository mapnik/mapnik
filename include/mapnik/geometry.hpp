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
    enum GeomType {
      Point = 1,
      LineString = 2,
      Polygon = 3
    };
    
    template <typename T>
    class geometry : private boost::noncopyable
    {   
    public:
        typedef T vertex_type;
        typedef typename vertex_type::type value_type;
    public:
        geometry () {}  
  
        Envelope<double> envelope() const
        {
            Envelope<double> result;    
            double x,y;
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
        
        virtual int type() const=0;
        virtual bool hit_test(value_type x,value_type y, double tol) const=0;  
        virtual void label_position(double *x, double *y) const=0;
        virtual void move_to(value_type x,value_type y)=0;
        virtual void line_to(value_type x,value_type y)=0;
        virtual unsigned num_points() const = 0;
        virtual unsigned vertex(double* x, double* y) const=0;
        virtual void rewind(unsigned) const=0;
        virtual void set_capacity(size_t size)=0;
        virtual ~geometry() {}
    };
    
   template <typename T>
   class point : public geometry<T>
   {
         typedef geometry<T> geometry_base;
         typedef typename geometry<T>::vertex_type vertex_type;
         typedef typename geometry<T>::value_type value_type;
      private:
         vertex_type pt_;
      public:
         point() : geometry<T>() {}
         
         int type() const 
         {
            return Point;
         }
         
         void label_position(double *x, double *y) const
         {
            *x = pt_.x;
            *y = pt_.y;
         }
         
         void move_to(value_type x,value_type y)
         {
            pt_.x = x;
            pt_.y = y;
         }
         
         void line_to(value_type ,value_type ) {}
         
         unsigned num_points() const
         {
            return 1;
         }
         
         unsigned vertex(double* x, double* y) const
         {
            *x = pt_.x;
            *y = pt_.y;
            return SEG_LINETO;
         }
         
         void rewind(unsigned ) const {}
         
         bool hit_test(value_type x,value_type y, double tol) const
         {
            return point_in_circle(pt_.x,pt_.y, x,y,tol);
         }
         
         void set_capacity(size_t) {}
         virtual ~point() {}
   };
   
   template <typename T, template <typename> class Container=vertex_vector2>
   class polygon : public geometry<T>
   {
         typedef geometry<T> geometry_base;
         typedef typename geometry<T>::vertex_type vertex_type;
         typedef typename geometry_base::value_type value_type;
         typedef Container<vertex_type> container_type;
      private:
         container_type cont_;
         mutable unsigned itr_;
      public:
         polygon()
            : geometry_base(),
              itr_(0)
         {}
         
         int type() const 
         {
            return Polygon;
         }
         
         void label_position(double *x, double *y) const
         {
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
         
         void line_to(value_type x,value_type y)
         {
            cont_.push_back(x,y,SEG_LINETO);
         }
         
         void move_to(value_type x,value_type y)
         {
            cont_.push_back(x,y,SEG_MOVETO);
         }
         
         unsigned num_points() const
         {
            return cont_.size();
         }
         
         unsigned vertex(double* x, double* y) const
         {
            return cont_.get_vertex(itr_++,x,y);
         }
         
         void rewind(unsigned ) const
         {
            itr_=0;
         }
         
         bool hit_test(value_type x,value_type y, double) const
         {      
            return point_inside_path(x,y,cont_.begin(),cont_.end());
         } 
         
         void set_capacity(size_t size) 
         {
            cont_.set_capacity(size);
         }
         virtual ~polygon() {}
   };
   
   template <typename T, template <typename> class Container=vertex_vector2>
   class line_string : public geometry<T>
   {
         typedef geometry<T> geometry_base;
         typedef typename geometry_base::value_type value_type;
         typedef typename geometry<T>::vertex_type vertex_type;
         typedef Container<vertex_type> container_type;
      private:
         container_type cont_;
         mutable unsigned itr_;
      public:
         line_string()
            : geometry_base(),
              itr_(0)
         {}
         
         int type() const 
         {
            return LineString;
         }
         void label_position(double *x, double *y) const
         {
            // calculate mid point on line string
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
         void line_to(value_type x,value_type y)
         {
            cont_.push_back(x,y,SEG_LINETO);
         }
         
         void move_to(value_type x,value_type y)
         {
            cont_.push_back(x,y,SEG_MOVETO);
         }
         
         unsigned num_points() const
         {
            return cont_.size();
         }
         
         unsigned vertex(double* x, double* y) const
         {
            return cont_.get_vertex(itr_++,x,y);
         }
         
         void rewind(unsigned ) const
         {
            itr_=0;
         }
         
         bool hit_test(value_type x,value_type y, double tol) const
         {      
            return point_on_path(x,y,cont_.begin(),cont_.end(),tol);
         } 
         
         void set_capacity(size_t size) 
         {
            cont_.set_capacity(size);
         }
         virtual ~line_string() {}
   };
   
   typedef point<vertex2d> point_impl;
   typedef line_string<vertex2d,vertex_vector2> line_string_impl;
   typedef polygon<vertex2d,vertex_vector2> polygon_impl;
   
   typedef geometry<vertex2d> geometry2d;
   typedef boost::shared_ptr<geometry2d> geometry_ptr;
   typedef boost::ptr_vector<geometry2d> geometry_containter;
}

#endif //GEOMETRY_HPP
