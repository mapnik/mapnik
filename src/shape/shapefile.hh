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

#ifndef SHAPEFILE_HH
#define SHAPEFILE_HH

#include "mapnik.hh"
#include <fstream>

using namespace mapnik;

class shape_file
{
    // POD structure to hold shape record 
    struct shape_record
    {
	unsigned char* data;
	size_t size;
	explicit shape_record(size_t size)
	    : data(static_cast<unsigned char*>(::operator new(sizeof(unsigned char)*size))),
	      size(size) {}
	~shape_record() 
	{
	    ::operator delete(data);
	}
    };
    
    std::ifstream file_;
    static const int buffer_size = 16;
    char buff_[buffer_size];
public:
    shape_file();
    shape_file(const std::string& file_name);
    ~shape_file();
    bool open(const std::string& file_name);
    bool is_open();
    void close();
    
    template <typename T,int dim>
    inline void shape_file::read_coord(coord<T,dim>& coord)
    {
#ifndef WORDS_BIGENDIAN
	file_.read(reinterpret_cast<char*>(&coord),sizeof(coord));
#else
#error "TODO: big-endian"
#endif
    }

    template <typename T>
    inline void shape_file::read_coords(coord_array<T> &ar)
    {
#ifndef WORDS_BIGENDIAN
	file_.read(reinterpret_cast<char*>(&ar[0]),sizeof(T)*ar.size());  
#else
#error "TODO: big-endian"
#endif    
	
    }

    inline int read_xdr_integer()
    {
#ifndef WORDS_BIGENDIAN
	char b[4];
	file_.read(b, 4);
	return b[3] & 0xffu | (b[2] & 0xffu) << 8 |
	    (b[1] & 0xffu) << 16 | (b[0] & 0xffu) << 24;
#else
#error "TODO: big-endian "
#endif
    }

    inline int read_ndr_integer()
    {
#ifndef WORDS_BIGENDIAN
	char b[4];
	file_.read(b,4);
	return b[0]&0xffu | (b[1]&0xffu) << 8 | 
	    (b[2]&0xffu) << 16 | (b[3]&0xffu) << 24;
#else
#error "TODO: big-endian "
#endif
    }

    inline double read_double()
    {
#ifndef WORDS_BIGENDIAN
	double val;
	file_.read(reinterpret_cast<char*>(&val),sizeof(val));
	return val;
#else
#error "TODO: big-endian "
#endif
    }

    inline void read_envelope(Envelope<double>& envelope)
    {
#ifndef WORDS_BIGENDIAN
	file_.read(reinterpret_cast<char*>(&envelope),sizeof(envelope));
#else
#error "TODO: big-endian"
#endif  
    }


    inline void skip(int bytes)
    {
	file_.seekg(bytes,std::ios::cur);
    }


    inline void rewind()
    {
	seek(100);
    }


    inline void seek(long pos)
    {
	file_.seekg(pos,std::ios::beg);
    }


    inline long pos()
    {
	return file_.tellg();
    }


    inline bool is_eof()
    {
	return file_.eof();
    }
    
private:
    shape_file(const shape_file&);
    shape_file& operator=(const shape_file&);
};
#endif                                            //SHAPEFILE_HH
