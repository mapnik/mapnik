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

//$Id: shapefile.hpp 33 2005-04-04 13:01:03Z pavlenko $

#ifndef SHAPEFILE_HPP
#define SHAPEFILE_HPP

#include <mapnik/envelope.hpp>
// boost
#include <boost/utility.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#include <cstring>

using mapnik::Envelope;

struct shape_record
{
      const char* data;
      size_t size;
      mutable size_t pos;
      explicit shape_record(size_t size)
         : size(size),
           pos(0) {} 
      
      void set_data(const char * data_)
      {
         data = data_;
      }
      
      void skip(unsigned n)
      {
         pos+=n;
      }
      
      int read_ndr_integer()
      {
         int val=(data[pos] & 0xff)     | 
            (data[pos+1] & 0xff) << 8  |
            (data[pos+2] & 0xff) << 16 |
            (data[pos+3] & 0xff) << 24;
         pos+=4;
         return val;
      }
      
      int read_xdr_integer()
      {
         int val=(data[pos] & 0xff) << 24 | 
            (data[pos+1] & 0xff)   << 16   |
            (data[pos+2] & 0xff)   << 8    |
            (data[pos+3] & 0xff);
         pos+=4;
         return val;
      }
      
      double read_double()
      {
         double val;		
#ifndef WORDS_BIGENDIAN
         std::memcpy(&val,&data[pos],8);	
#else
         long long bits = ((long long)data[pos] & 0xff) | 
            ((long long)data[pos+1] & 0xff) << 8   |
            ((long long)data[pos+2] & 0xff) << 16  |
            ((long long)data[pos+3] & 0xff) << 24  |
            ((long long)data[pos+4] & 0xff) << 32  |
            ((long long)data[pos+5] & 0xff) << 40  |
            ((long long)data[pos+6] & 0xff) << 48  |
            ((long long)data[pos+7] & 0xff) << 56  ;
         std::memcpy(&val,&bits,8);
#endif 
         pos+=8;
         return val;
      }
      long remains() 
      {
         return (size-pos);
      }
      ~shape_record() {}
 
};

using namespace boost::iostreams;

class shape_file : boost::noncopyable
{  
      stream<mapped_file_source> file_;
   public:
      shape_file();
      shape_file(const std::string& file_name);
      ~shape_file();
      bool is_open();
      void close();
      
      inline void read_record(shape_record& rec)
      {
         rec.set_data(file_->data() + file_.tellg());
         file_.seekg(rec.size,std::ios::cur);
      }
      
      inline int read_xdr_integer()
      {
         char b[4];
         file_.read(b, 4);
         return (b[3] & 0xffu) | ((b[2] & 0xffu) << 8) |
            ((b[1] & 0xffu) << 16) | ((b[0] & 0xffu) << 24);
      }
      
      inline int read_ndr_integer()
      {
         char b[4];
         file_.read(b,4);
         return (b[0]&0xffu) | ((b[1]&0xffu) << 8) | 
            ((b[2]&0xffu) << 16) | ((b[3]&0xffu) << 24);
      }
      
      inline double read_double()
      {
         double val;
#ifndef WORDS_BIGENDIAN
         file_.read(reinterpret_cast<char*>(&val),8);
#else
         char b[8];
         file_.read(b,8);
         long long bits = ((long long)b[0] & 0xff) | 
            ((long long)b[1] & 0xff) << 8   |
            ((long long)b[2] & 0xff) << 16  |
            ((long long)b[3] & 0xff) << 24  |
            ((long long)b[4] & 0xff) << 32  |
            ((long long)b[5] & 0xff) << 40  |
            ((long long)b[6] & 0xff) << 48  |
            ((long long)b[7] & 0xff) << 56  ;
         memcpy(&val,&bits,8);
#endif
         return val;
      }
      
      inline void read_envelope(Envelope<double>& envelope)
      {
#ifndef WORDS_BIGENDIAN
         file_.read(reinterpret_cast<char*>(&envelope),sizeof(envelope));
#else
         double minx=read_double();
         double miny=read_double();
         double maxx=read_double();
         double maxy=read_double();
         envelope.init(minx,miny,maxx,maxy);
#endif  
      }
      
      inline void skip(std::streampos bytes)
      {
         file_.seekg(bytes,std::ios::cur);
      }
      
      inline void rewind()
      {
         seek(100);
      }
      
      inline void seek(std::streampos pos)
      {
         file_.seekg(pos,std::ios::beg);
      }
      
      inline std::streampos pos()
      {
         return file_.tellg();
      }
      
      inline bool is_eof()
      {
         return file_.eof();
      }
};

#endif //SHAPEFILE_HPP
