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

#include <mapnik/global.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/feature.hpp>

// boost
#include <boost/utility.hpp>

namespace mapnik
{
    struct wkb_reader : boost::noncopyable
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
        wkbFormat format_;

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
	
        wkb_reader(const char* wkb,unsigned size,wkbFormat format)
            : wkb_(wkb),
              size_(size),
              pos_(0),
              format_(format)
        {
            switch (format_)
            {
            case wkbSpatiaLite:
                byteOrder_ = (wkbByteOrder) wkb_[1];
                pos_ = 39;
                break;

            case wkbGeneric:
            default:
                byteOrder_ = (wkbByteOrder) wkb_[0];
                pos_ = 1;
                break;
            }

#ifndef MAPNIK_BIG_ENDIAN
            needSwap_=byteOrder_?wkbXDR:wkbNDR;
#else
            needSwap_=byteOrder_?wkbNDR:wkbXDR;	
#endif	    
        }

        ~wkb_reader() {}

        void read_multi(Feature & feature) 
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
                read_multipoint_2(feature);
                break;
            case wkbMultiLineString:
                read_multilinestring_2(feature);
                break;
            case wkbMultiPolygon:
                read_multipolygon_2(feature);
                break;
            case wkbGeometryCollection:
                break;
            default:
                break;
            }
        }
          
    private:
        
        int read_integer() 
        {
	    boost::int32_t n;
            if (needSwap_)
            {
                read_int32_xdr(wkb_+pos_,n);
            } 
            else 
            {
                read_int32_ndr(wkb_+pos_,n);
            }
            pos_+=4;
            
            return n;
        }
	
        double read_double()
        {
            double d;
            if (needSwap_)
            {
                read_double_xdr(wkb_ + pos_, d);
            }
            else 
            {
                read_double_ndr(wkb_ + pos_, d);
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
                pos_+=size;
            }
            else 
            {
                for (unsigned i=0;i<ar.size();++i)
                {
                    read_double_xdr(wkb_ + pos_,ar[i].x);
                    read_double_xdr(wkb_ + pos_ + 8,ar[i].y);
                    pos_ += 16;
                }
            }
            
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
         
        void read_multipoint_2(Feature & feature)
        {
            geometry2d * pt = new point<vertex2d>;
            int num_points = read_integer(); 
            for (int i=0;i<num_points;++i) 
            {
                pos_+=5;
                double x = read_double();
                double y = read_double();
                pt->move_to(x,y);
            }
            feature.add_geometry(pt);
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

        void read_multilinestring_2(Feature & feature)
        {
            geometry2d * line = new line_string<vertex2d>;
            int num_lines=read_integer();
            unsigned capacity = 0;
            for (int i=0;i<num_lines;++i)
            {
                pos_+=5;
                int num_points=read_integer();
                capacity+=num_points;
                CoordinateArray ar(num_points); 
                read_coords(ar);
                line->set_capacity(capacity);
                line->move_to(ar[0].x,ar[0].y); 
                for (int i=1;i<num_points;++i) 
                { 
                    line->line_to(ar[i].x,ar[i].y); 
                } 
            }
            feature.add_geometry(line);
        }
         
        void read_polygon(Feature & feature) 
        {
            geometry2d * poly = new polygon<vertex2d>;
            int num_rings=read_integer();
            unsigned capacity = 0;
            for (int i=0;i<num_rings;++i)
            {
                int num_points=read_integer();
                capacity+=num_points;
                CoordinateArray ar(num_points);
                read_coords(ar);
                poly->set_capacity(capacity);
                poly->move_to(ar[0].x,ar[0].y);
                for (int j=1;j<num_points;++j)
                {
                    poly->line_to(ar[j].x,ar[j].y);
                }
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

        void read_multipolygon_2(Feature & feature)
        {
            geometry2d * poly = new polygon<vertex2d>;
            int num_polys=read_integer();
            unsigned capacity = 0;
            for (int i=0;i<num_polys;++i)
            {
                pos_+=5;
                int num_rings=read_integer();
                for (int i=0;i<num_rings;++i)
                {
                    int num_points=read_integer();
                    capacity += num_points;
                    CoordinateArray ar(num_points);
                    read_coords(ar);
                    poly->set_capacity(capacity);
                    poly->move_to(ar[0].x,ar[0].y);
                  
                    for (int j=1;j<num_points;++j)
                    {
                        poly->line_to(ar[j].x,ar[j].y);
                    }
                    poly->line_to(ar[0].x,ar[0].y);
                }
            }
            feature.add_geometry(poly);
        }
    };
    
    void geometry_utils::from_wkb (Feature & feature,
                                   const char* wkb,
                                   unsigned size,
                                   bool multiple_geometries,
                                   wkbFormat format) 
    {
        wkb_reader reader(wkb,size,format);
        if (multiple_geometries)
            return reader.read_multi(feature);
        else
            return reader.read(feature);
    }    
}
