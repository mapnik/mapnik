/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 CartoDB
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

// This is the WKB contract, we should look like this.
// static bool from_wkb (boost::ptr_vector<geometry_type>& paths,
//                       const char* wkb,
//                       unsigned size,
//                       wkbFormat format = wkbGeneric);



#ifndef POSTGIS_UTILS_CPP
#define POSTGIS_UTILS_CPP

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/global.hpp>
#include <mapnik/coord_array.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/noncopyable.hpp>

// boost
#include <boost/format.hpp>

// plugin
#include "postgis_utils.hpp"


namespace mapnik
{

typedef coord_array<coord2d> CoordinateArray;

struct twkb_reader : mapnik::noncopyable
{
private:

    const char* twkb_;
    size_t size_;
    unsigned int pos_;

    /* Metadata on the geometry we are parsing */
	uint32_t twkb_type_; 
	uint8_t has_bbox_;
	uint8_t has_size_;
	uint8_t has_idlist_;
	uint8_t has_z_;
	uint8_t has_m_;
	uint8_t is_empty_;

	/* Precision factors to convert ints to double */
	double factor_xy_;
	double factor_z_;
	double factor_m_;

    /* An array to keep delta values from 4 dimensions */
	int64_t coord_x_; 
	int64_t coord_y_; 
	int64_t coord_z_; 
	int64_t coord_m_; 


public:

    enum twkbGeometryType {
        twkbPoint=1,
        twkbLineString=2,
        twkbPolygon=3,
        twkbMultiPoint=4,
        twkbMultiLineString=5,
        twkbMultiPolygon=6,
        twkbGeometryCollection=7
    };
    
    
    twkb_reader(const char* twkb, size_t size)
        : twkb_(twkb),
          size_(size),
          pos_(0),
          twkb_type_(0),   // Geometry type 
          has_bbox_(0),    // Bounding box? 
          has_size_(0),    // Size attribute? 
          has_idlist_(0),  // Presence of X/Y  
          has_z_(0),       // Presence of Z 
          has_m_(0),       // Presence of M 
          is_empty_(0),    // Empty? 
          factor_xy_(0.0), // Expansion factor for X/Y 
          factor_z_(0.0),  // Expansion factor for Z 
          factor_m_(0.0)   // Expansion factor for M 
    {

    }    

    void read(boost::ptr_vector<geometry_type> & paths)
    {
        // Read the metadata bytes, populating all the 
        // information about optional fields, extended (z/m) dimensions
        // expansion factors and so on
        read_header();
        
        // Each new read call has to reset the coordinate accumulators
        coord_x_ = 0;     // Accumulation register (x)
        coord_y_ = 0;     // Accumulation register (y)
        coord_z_ = 0;     // Accumulation register (z)
        coord_m_ = 0;     // Accumulation register (m)        
        
        // If the geometry is empty, add nothing to the paths array
        if ( is_empty_ )
            return;
        
        // Read the [optional] size information
    	if ( has_size_ )
    		size_ = read_unsigned_integer();
        
        // Read the [optional] bounding box information
        if ( has_bbox_ )
            read_bbox();
    
        switch (twkb_type_)
        {
            case twkbPoint:
                read_point(paths);
                break;
            case twkbLineString:
                read_linestring(paths);
                break;
            case twkbPolygon:
                read_polygon(paths);
                break;
            case twkbMultiPoint:
                read_multipoint(paths);
                break;
            case twkbMultiLineString:
                read_multilinestring(paths);
                break;
            case twkbMultiPolygon:
                read_multipolygon(paths);
                break;
            case twkbGeometryCollection:
                read_collection(paths);
            default:
                return;
        }
    }
    
private:
    
    int64_t unzigzag64(uint64_t val)
    {
            if ( val & 0x01 ) 
                return -1 * (int64_t)((val+1) >> 1);
            else
                return (int64_t)(val >> 1);
    }
	
    int32_t unzigzag32(uint32_t val)
    {
            if ( val & 0x01 ) 
                return -1 * (int32_t)((val+1) >> 1);
            else
                return (int32_t)(val >> 1);
    }
	
    int8_t unzigzag8(uint8_t val)
    {
            if ( val & 0x01 ) 
                return -1 * (int8_t)((val+1) >> 1);
            else
                return (int8_t)(val >> 1);
    }
    
    /* Read from signed 64bit varint */
    int64_t read_signed_integer()
    {	
    	return unzigzag64(read_unsigned_integer());
    }
    
    /* Read from unsigned 64bit varint */
    uint64_t read_unsigned_integer()
    {
    	uint64_t nVal = 0;
    	int nShift = 0;
    	uint8_t nByte;

    	/* Check so we don't read beyond the twkb */
    	while ( pos_ < size_ )
    	{
    		nByte = twkb_[pos_];

    		/* We get here when there is more to read in the input varInt */
    		/* Here we take the least significant 7 bits of the read */
    		/* byte and put it in the most significant place in the result variable. */
    		nVal |= ((uint64_t)(nByte & 0x7f)) << nShift;

    		/* move the "cursor" of the input buffer step (8 bits) */
    		pos_++;

    		/* move the cursor in the resulting variable (7 bits) */
    		nShift += 7;

    		/* Hibit isn't set, so this is the last byte */
    		if ( !(nByte & 0x80) )
    		{
    			return nVal;
    		}
    	}
                
        // lwerror("%s: varint extends past end of buffer", __func__);
    	return 0;
    }

    // Every TWKB geometry starts with a metadata header
    // 
    // type_and_dims     byte
    // metadata_header   byte
    // [extended_dims]   byte
    // [size]            uvarint
    // [bounds]          bbox
    // 
    void read_header()
    {
        uint8_t type_precision = twkb_[pos_++];
        uint8_t metadata = twkb_[pos_++];
    	twkb_type_ = type_precision & 0x0F;
    	int8_t precision = unzigzag8((type_precision & 0xF0) >> 4);
    	factor_xy_  = pow(10, (double)precision);
    	has_bbox_   =  metadata & 0x01;
    	has_size_   = (metadata & 0x02) >> 1;
    	has_idlist_ = (metadata & 0x04) >> 2;
    	uint8_t zm  = (metadata & 0x08) >> 3;
    	is_empty_   = (metadata & 0x10) >> 4;
        
    	// Flag for higher dimensions means read a third byte 
        // of extended dimension information
    	if ( zm )
    	{
    		zm = twkb_[pos_++];

    		/* Strip Z/M presence and precision from ext byte */
    		has_z_  = (zm & 0x01);
    		has_m_  = (zm & 0x02) >> 1;

    		/* Convert the precision into factor */
    		int8_t precision_z = (zm & 0x1C) >> 2;
    		int8_t precision_m = (zm & 0xE0) >> 5;
    		factor_z_ = pow(10, (double)precision_z);
    		factor_m_ = pow(10, (double)precision_m);
    	}
        
    }

    void read_bbox()
    {
        // we have nowhere to store this box information
        // for now, so we'll just move the read head forward
        // an appropriate number of times
        if ( has_bbox_ )
        {
            read_signed_integer(); // uint64_t xmin
            read_signed_integer(); // uint64_t xdelta
            read_signed_integer(); // uint64_t ymin
            read_signed_integer(); // uint64_t ydelta
            if ( has_z_ )
            {
                read_signed_integer(); // uint64_t zmin
                read_signed_integer(); // uint64_t zdelta
            }
            if ( has_m_ )
            {
                read_signed_integer(); // uint64_t mmin
                read_signed_integer(); // uint64_t mdelta
            }
        }
    }
    
    void read_idlist(unsigned int num_ids) 
    {
        // we have nowhere to store this id information
        // for now, so we'll just move the read head
        // forward an appropriate number of times
        if ( has_idlist_ )
        {
            for ( unsigned int i = 0; i < num_ids; i++ )
            {
                read_signed_integer(); // uint64_t id
            }
        }
    }
    
    void read_coords(CoordinateArray& ar)
    {
        for ( unsigned int i = 0; i < ar.size(); i++ )
    	{
    		// X
            coord_x_ += read_signed_integer();
            ar[i].x = coord_x_ / factor_xy_;

    		// Y
            coord_y_ += read_signed_integer();
            ar[i].y = coord_y_ / factor_xy_;

            // No Mapnik slot to hold Z, so just move read head forward 
            if ( has_z_ )
                coord_z_ += read_signed_integer();

            // No Mapnik slot to hold M, so just move read head forward 
    		if ( has_m_ )
    			coord_m_ += read_signed_integer();
    	}
        
    }

    void read_point(boost::ptr_vector<geometry_type> & paths)
    {
        CoordinateArray ar(1); // CoordinateArray of length one
        read_coords(ar);
        std::auto_ptr<geometry_type> pt(new geometry_type(Point));
        pt->move_to(ar[0].x, ar[0].y);
        paths.push_back(pt);
    }

    void read_multipoint(boost::ptr_vector<geometry_type> & paths)
    {
        unsigned int num_points = read_unsigned_integer();
        
        if ( has_idlist_ )
            read_idlist(num_points);
        
        if (num_points > 0)
        {
            CoordinateArray ar(num_points);
            read_coords(ar);
            for ( unsigned int i = 0; i < ar.size(); ++i )
            {
                std::auto_ptr<geometry_type> pt(new geometry_type(Point));
                pt->move_to(ar[i].x, ar[i].y);
                paths.push_back(pt);
            }
        }
    }

    void read_linestring(boost::ptr_vector<geometry_type> & paths)
    {
        unsigned int num_points = read_unsigned_integer();
        if (num_points > 0)
        {
            CoordinateArray ar(num_points);
            read_coords(ar);
            std::auto_ptr<geometry_type> line(new geometry_type(LineString));
            line->move_to(ar[0].x, ar[0].y);
            for ( unsigned int i = 1; i < ar.size(); ++i )
            {
                line->line_to(ar[i].x, ar[i].y);
            }
            paths.push_back(line);
        }
    }
    
    void read_multilinestring(boost::ptr_vector<geometry_type> & paths)
    {
        unsigned int num_lines = read_unsigned_integer();
        
        if ( has_idlist_ )
            read_idlist(num_lines);
        
        for ( unsigned int i = 0; i < num_lines; ++i )
        {
            read_linestring(paths);
        }
    }

    void read_polygon(boost::ptr_vector<geometry_type> & paths)
    {
        unsigned int num_rings = read_unsigned_integer();
        if (num_rings > 0)
        {
            std::auto_ptr<geometry_type> poly(new geometry_type(Polygon));
            for ( unsigned int i = 0; i < num_rings; ++i )
            {
                int num_points = read_unsigned_integer();
                if (num_points > 0)
                {
                    CoordinateArray ar(num_points);
                    read_coords(ar);
                    poly->move_to(ar[0].x, ar[0].y);
                    for (int j = 1; j < num_points ; ++j)
                    {
                        poly->line_to(ar[j].x, ar[j].y);
                    }
                    poly->close_path();
                }
            }
            if (poly->size() > 3) // ignore if polygon has less than (3 + close_path) vertices
                paths.push_back(poly);
        }
    }

    void read_multipolygon(boost::ptr_vector<geometry_type> & paths)
    {
        unsigned int num_polys = read_unsigned_integer();

        if ( has_idlist_ )
            read_idlist(num_polys);
        
        for ( unsigned int i = 0; i < num_polys; ++i )
        {
            read_polygon(paths);
        }
    }

    void read_collection(boost::ptr_vector<geometry_type> & paths)
    {
        unsigned int num_geometries = read_unsigned_integer();
        
        if ( has_idlist_ )
            read_idlist(num_geometries);
        
        for ( unsigned int i = 0; i < num_geometries; ++i )
        {
            read(paths);
        }
    }


};


bool postgis_utils::from_twkb(boost::ptr_vector<geometry_type>& paths,
                              const char* twkb,
                              unsigned size)
{
    unsigned geom_count = paths.size();
    twkb_reader reader(twkb, size);
    reader.read(paths);
    if (paths.size() > geom_count)
        return true;
    return false;
}



}


#endif // POSTGIS_UTILS_CPP
