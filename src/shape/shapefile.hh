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

#ifndef SHAPEFILE_HH
#define SHAPEFILE_HH

#include "mapnik.hh"
#include <fstream>

using namespace mapnik;

class shape_file
{
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
    int read_xdr_integer();
    int read_ndr_integer();
    double read_double();

    template <typename T,int dim>
    void shape_file::read_coord(coord<T,dim>& coord)
    {
#ifndef WORDS_BIGENDIAN
	file_.read(reinterpret_cast<char*>(&coord),sizeof(coord));
#else
#error "TODO: big-endian"
#endif
    }

    template <typename T>
    void shape_file::read_coords(coord_array<T> &ar)
    {
#ifndef WORDS_BIGENDIAN
	file_.read(reinterpret_cast<char*>(&ar[0]),sizeof(T)*ar.size());  
#else
#error "TODO: big-endian"
#endif    
	
    }

    void read_envelope(Envelope<double> &envelope);
    void skip(int bytes);
    void rewind();
    void seek(long pos);
    long pos();
    bool is_eof();
private:
    shape_file(const shape_file&);
    shape_file& operator=(const shape_file&);
};
#endif                                            //SHAPEFILE_HH
