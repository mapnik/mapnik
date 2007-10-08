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

//$Id: wkb.cpp 19 2005-03-22 13:53:27Z pavlenko $

#include <mapnik/wkb.hpp>

#include <mapnik/geom_util.hpp>
#include <mapnik/feature.hpp>

namespace mapnik
{
   struct wkb_reader
   {
      private:
         enum wkbByteOrder {
            wkbXDR=0,
            wkbNDR=1
         };
         const char* wkb_;
         unsigned size_;
         unsigned pos_;
         wkbByteOrder byteOrder_;
         bool needSwap_;
      public:
	
         enum wkbGeometryType {
            wkbPoint=1,
            wkbLineString=2,
            wkbPolygon=3,
            wkbMultiPoint=4,
            wkbMultiLineString=5,
            wkbMultiPolygon=6,
            wkbGeometryCollection=7
         };
	
         wkb_reader(const char* wkb,unsigned size)
            : wkb_(wkb),
              size_(size),
              pos_(0),
              byteOrder_((wkbByteOrder)wkb_[0])
         {
            ++pos_;
	    
#ifndef WORDS_BIGENDIAN
            needSwap_=byteOrder_?wkbXDR:wkbNDR;
#else
            needSwap_=byteOrder_?wkbNDR:wkbXDR;	
#endif	    
         }

         ~wkb_reader() {}

         void read(Feature & feature) 
         {
            int type=read_integer();
            switch (type)
            {
               case wkbPoint:
                  read_point(feature);
                  break;
               case wkbLineString:
                  read_linestring(feature);
                  break;
               case wkbPolygon:
                  read_polygon(feature);
                  break;
               case wkbMultiPoint:
                  read_multipoint(feature);
                  break;
               case wkbMultiLineString:
                  read_multilinestring(feature);
                  break;
               case wkbMultiPolygon:
                  read_multipolygon(feature);
                  break;
               case wkbGeometryCollection:
                  break;
               default:
                  break;
            }
         }
          
      private:
         wkb_reader(const wkb_reader&);
         wkb_reader& operator=(const wkb_reader&);
	
         int read_integer() 
         {
            int n;

            if (!needSwap_)
            {
               memcpy(&n,wkb_+pos_,4);
            } 
            else 
            {
               const char* b=wkb_+pos_;
               n = b[3]&0xff | (b[2]&0xff)<<8 | (b[1]&0xff)<<16 | (b[0]&0xff)<<24;
            }
            pos_+=4;

            return n;
         }
	
         double read_double()
         {
            double d;

            if (!needSwap_)
            {
               memcpy(&d,wkb_+pos_,8);
            }
            else 
            {
               // we rely on the fact that "long long" is in C standard,
               // but not in C++ yet
               // this is not quite portable
               const char* b= wkb_+pos_;
               long long n = (long long)b[7]&0xff | 
                  ((long long)b[6]&0xff)<<8 | 
                  ((long long)b[5]&0xff)<<16 | 
                  ((long long)b[4]&0xff)<<24 |
                  ((long long)b[3]&0xff)<<32 |
                  ((long long)b[2]&0xff)<<40 |
                  ((long long)b[1]&0xff)<<48 |
                  ((long long)b[0]&0xff)<<56;
               memcpy(&d,&n,8);
            }
            pos_+=8;

            return d;
         }
	
         void read_coords(CoordinateArray& ar)
         {
            int size=sizeof(coord<double,2>)*ar.size();
            if (!needSwap_)
            {
               std::memcpy(&ar[0],wkb_+pos_,size);
		
            }
            else 
            {
               for (unsigned i=0;i<ar.size();++i)
               {
                  ar[i].x=read_double();
                  ar[i].y=read_double();
               }
            }
            pos_+=size;
         }
	
         void read_point(Feature & feature)
         {
            geometry2d * pt = new point<vertex2d>;
            double x = read_double();
            double y = read_double();
            pt->move_to(x,y);
            feature.add_geometry(pt);
         }
         
         void read_multipoint(Feature & feature)
         {
            int num_points = read_integer();
            for (int i=0;i<num_points;++i) 
            {
               pos_+=5;
               read_point(feature);
            }
         }
         
         void read_linestring(Feature & feature)
         {
            geometry2d * line = new line_string<vertex2d>;
            int num_points=read_integer();
            CoordinateArray ar(num_points);
            read_coords(ar);
            line->set_capacity(num_points);
            line->move_to(ar[0].x,ar[0].y);
            for (int i=1;i<num_points;++i)
            {
               line->line_to(ar[i].x,ar[i].y);
            }
            feature.add_geometry(line);
         }
         
         void read_multilinestring(Feature & feature)
         {
            int num_lines=read_integer();
            for (int i=0;i<num_lines;++i)
            {
               pos_+=5;
               read_linestring(feature);
            }
         }

         void read_polygon(Feature & feature) 
         {
            geometry2d * poly = new polygon<vertex2d>;
            int num_rings=read_integer();
            for (int i=0;i<num_rings;++i)
            {
               int num_points=read_integer();
               CoordinateArray ar(num_points);
               read_coords(ar);
               poly->move_to(ar[0].x,ar[0].y);

               for (int j=1;j<num_points;++j)
               {
                  poly->line_to(ar[j].x,ar[j].y);
               }
               poly->line_to(ar[0].x,ar[0].y);
            }
            feature.add_geometry(poly);
         }
	
         void read_multipolygon(Feature & feature)
         {
            int num_polys=read_integer();
            for (int i=0;i<num_polys;++i)
            {
               pos_+=5;
               read_polygon(feature);
            }
         }
   };
   
   void geometry_utils::from_wkb(Feature & feature,const char* wkb, unsigned size) 
   {
      wkb_reader reader(wkb,size);
      return reader.read(feature);
   }    
}
