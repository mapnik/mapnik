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

//$Id: wkb.cc 58 2004-10-31 16:21:26Z artem $

#include "wkb.hh"
#include "geom_util.hh"

namespace mapnik
{

    struct wkb_reader
    {
    private:
	const char* wkb_;
	unsigned size_;
	int srid_;
	unsigned pos_;
	unsigned byteOrder_;
    public:

	enum wkbByteOrder {
	    wkbNDR=0,
	    wkbXDR=1
	};
	
	enum wkbGeometryType {
	    wkbPoint=1,
	    wkbLineString=2,
	    wkbPolygon=3,
	    wkbMultiPoint=4,
	    wkbMultiLineString=5,
	    wkbMultiPolygon=6,
	    wkbGeometryCollection=7
	};
	
	wkb_reader(const char* wkb,unsigned size,int srid)
	    : wkb_(wkb),
	      size_(size),
	      srid_(srid),
	      pos_(0),
	      byteOrder_(wkb_[0]) 
	{
	    ++pos_;
	}

	~wkb_reader() {}

	geometry_ptr read() 
	{
	    geometry_ptr geom;
	    int type=read_integer();
	    switch (type)
	    {
            case wkbPoint:
		geom = read_point();
                break;
            case wkbLineString:
                geom = read_linestring();
                break;
            case wkbPolygon:
                geom = read_polygon();
                break;
            case wkbMultiPoint:
		geom = read_multipoint();
                break;
            case wkbMultiLineString:
                geom = read_multilinestring();
                break;
            case wkbMultiPolygon:
                geom=read_multipolygon();
                break;
            case wkbGeometryCollection:
                break;
            default:
                break;
	    }
	    return geom;
	}
	
    private:
	wkb_reader(const wkb_reader&);
	wkb_reader& operator=(const wkb_reader&);
	
	int read_integer() 
	{
	    int n;
	    memcpy(&n,wkb_+pos_,4);
	    pos_+=4;
	    return n;
	}
	
	double read_double()
	{
	    double d;
	    memcpy(&d,wkb_+pos_,8);
	    pos_+=8;
	    return d;
	}
	
	void read_coords(CoordinateArray& ar)
	{
	    int size=sizeof(coord<double,2>)*ar.size();
	    std::memcpy(&ar[0],wkb_+pos_,size);
	    pos_+=size;
	}
	
	geometry_ptr read_point()
	{
	    geometry_ptr pt(new point<vertex2d>(srid_));
	    double x = read_double();
	    double y = read_double();
	    pt->move_to(x,y);
	    return pt;
	}
	
	geometry_ptr read_multipoint()
	{
	    geometry_ptr pt(new point<vertex2d>(srid_));
	    int num_points = read_integer();
	    for (int i=0;i<num_points;++i) 
	    {
		pos_+=5; 
		double x = read_double();
		double y = read_double();
		pt->move_to(x,y);
	    }
	    return pt; 
	}

	geometry_ptr read_linestring()
	{
	    geometry_ptr line(new line_string<vertex2d>(srid_));
	    int num_points=read_integer();
	    CoordinateArray ar(num_points);
	    read_coords(ar);
	    line->move_to(ar[0].x,ar[0].y);
	    for (int i=1;i<num_points;++i)
	    {
		line->line_to(ar[i].x,ar[i].y);
	    }
	    return line;
	}

	geometry_ptr read_multilinestring()
	{
	    geometry_ptr line(new line_string<vertex2d>(srid_));
	    int num_lines=read_integer();

	    for (int i=0;i<num_lines;++i)
	    {
		pos_+=5;
		
		int num_points=read_integer();
		CoordinateArray ar(num_points);
		read_coords(ar);
		line->move_to(ar[0].x,ar[0].y);
		
		for (int i=1;i<num_points;++i)
		{
		    line->line_to(ar[i].x,ar[i].y);
		}
	    }
	    return line;
	}

	geometry_ptr read_polygon() 
	{
	    geometry_ptr poly(new polygon<vertex2d>(srid_));
	    
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
	    return poly;
	}
	
	geometry_ptr read_multipolygon()
	{
	    geometry_ptr poly(new polygon<vertex2d>(srid_));

	    int num_polys=read_integer();
	    for (int i=0;i<num_polys;++i)
	    {
		pos_+=5;
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
	    }
	    return poly;
        }
    };


    geometry_ptr geometry_utils::from_wkb(const char* wkb, unsigned size,int srid) 
    {
	wkb_reader reader(wkb,size,srid);
	return reader.read();
    }    
}
