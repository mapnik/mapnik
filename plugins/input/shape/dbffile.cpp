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
// mapnik
#include <mapnik/global.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/unicode.hpp>
#include "dbffile.hpp"
// boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

// stl
#include <string>


dbf_file::dbf_file()
   : num_records_(0),
     num_fields_(0),
     record_length_(0),
     record_(0) {}

dbf_file::dbf_file(std::string const& file_name)
   :num_records_(0),
    num_fields_(0),
    record_length_(0),
#ifdef SHAPE_MEMORY_MAPPED_FILE
    file_(file_name),
#else
    file_(file_name,std::ios::in | std::ios::binary),
#endif
    record_(0)
{
   if (file_.is_open())
   {
      read_header();
   }
}


dbf_file::~dbf_file()
{
   ::operator delete(record_);
}


bool dbf_file::is_open()
{
   return file_.is_open();
}


void dbf_file::close()
{
   if (file_ && file_.is_open())
      file_.close();
}


int dbf_file::num_records() const
{
   return num_records_;
}


int dbf_file::num_fields() const
{
   return num_fields_;
}


void dbf_file::move_to(int index)
{
   if (index>0 && index<=num_records_)
   {
      long pos=(num_fields_<<5)+34+(index-1)*(record_length_+1);
      file_.seekg(pos,std::ios::beg);
      file_.read(record_,record_length_);
   }
}


std::string dbf_file::string_value(int col) const
{
   if (col>=0 && col<num_fields_)
   {
      return std::string(record_+fields_[col].offset_,fields_[col].length_);
   }
   return "";
}


const field_descriptor& dbf_file::descriptor(int col) const
{
   assert(col>=0 && col<num_fields_);
   return fields_[col];
}


void dbf_file::add_attribute(int col, mapnik::transcoder const& tr, Feature const& f) const throw()
{
   if (col>=0 && col<num_fields_)
   {
      std::string name=fields_[col].name_;
      std::string str(record_+fields_[col].offset_,fields_[col].length_);
      boost::trim(str);
      
      switch (fields_[col].type_)
      {
         case 'C':
         case 'D'://todo handle date?
         case 'M':
         case 'L':
         {
            f[name] = tr.transcode(str.c_str()); 
            break;
         }
         case 'N':
         case 'F':
         {
            if (str[0]=='*')
            {
               boost::put(f,name,0);
               break;
            }
            if ( fields_[col].dec_>0 )
            {   
               double d = 0.0; 
               std::istringstream(str) >> d; 
               boost::put(f,name,d); 
            }
            else
            {
                int i = 0; 
                std::istringstream(str) >> i; 
                boost::put(f,name,i); 
            }
            break;
         }
      }
   }
}

void dbf_file::read_header()
{
   char c=file_.get();
   if (c=='\3' || c=='\131')
   {
      skip(3);
      num_records_=read_int();
      assert(num_records_>=0);
      num_fields_=read_short();
      assert(num_fields_>0);
      num_fields_=(num_fields_-33)/32;
      skip(22);
      int offset=0;
      char name[11];
      memset(&name,0,11);
      fields_.reserve(num_fields_);
      for (int i=0;i<num_fields_;++i)
      {
         field_descriptor desc;
         desc.index_=i;
         file_.read(name,10);
         desc.name_=boost::trim_left_copy(std::string(name));
         skip(1);
         desc.type_=file_.get();
         skip(4);
         desc.length_=file_.get();
         desc.dec_=file_.get();
         skip(14);
         desc.offset_=offset;
         offset+=desc.length_;
         fields_.push_back(desc);
      }
      record_length_=offset;
      if (record_length_>0)
      {
         record_=static_cast<char*>(::operator new (sizeof(char)*record_length_));
      }
   }
}


int dbf_file::read_short()
{
   char b[2];
   file_.read(b,2);
   boost::int16_t val;
   mapnik::read_int16_ndr(b,val);
   return val;
}


int dbf_file::read_int()
{    
   char b[4];
   file_.read(b,4);
   boost::int32_t val;
   mapnik::read_int32_ndr(b,val);
   return val;
}

void dbf_file::skip(int bytes)
{
   file_.seekg(bytes,std::ios::cur);
}
