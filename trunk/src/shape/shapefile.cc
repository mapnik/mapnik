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

#include "shapefile.hh"

shape_file::shape_file() {}


shape_file::shape_file(const std::string& file_name)
{
    file_.rdbuf()->pubsetbuf(buff_,buffer_size);
    file_.open(file_name.c_str(),std::ios::in|std::ios::binary);
}


shape_file::~shape_file()
{
    if (file_ && file_.is_open())
        file_.close();
}


bool shape_file::open(const std::string& file_name)
{
    file_.rdbuf()->pubsetbuf(buff_,buffer_size);
    file_.open(file_name.c_str(),std::ios::in | std::ios::binary);
    return file_?true:false;
}


bool shape_file::is_open()
{
    return file_.is_open();
}


void shape_file::close()
{
    if (file_ && file_.is_open())
        file_.close();
}


int shape_file::read_xdr_integer()
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

int shape_file::read_ndr_integer()
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


double shape_file::read_double()
{
#ifndef WORDS_BIGENDIAN
    double val;
    file_.read(reinterpret_cast<char*>(&val),sizeof(val));
    return val;
#else
#error "TODO: big-endian "
#endif
}

void shape_file::read_envelope(Envelope<double>& envelope)
{
#ifndef WORDS_BIGENDIAN
    file_.read(reinterpret_cast<char*>(&envelope),sizeof(envelope));
#else
#error "TODO: big-endian"
#endif  
}


void shape_file::skip(int bytes)
{
    file_.seekg(bytes,std::ios::cur);
}


void shape_file::rewind()
{
    seek(100);
}


void shape_file::seek(long pos)
{
    file_.seekg(pos,std::ios::beg);
}


long shape_file::pos()
{
    return file_.tellg();
}


bool shape_file::is_eof()
{
    return file_.eof();
}


