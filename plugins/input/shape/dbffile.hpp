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

#ifndef DBFFILE_HPP
#define DBFFILE_HPP

#include <mapnik/feature.hpp>
// boost
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
// stl
#include <vector>
#include <string>
#include <cassert>


using mapnik::transcoder;
using mapnik::Feature;

struct field_descriptor
{
    int index_;
    std::string name_;
    char type_;
    int length_;
    int dec_;
    int offset_;
};

using namespace boost::iostreams;

class dbf_file
{
private:
    int num_records_;
    int num_fields_;
    int record_length_;
    std::vector<field_descriptor> fields_;
#ifdef SHAPE_MEMORY_MAPPED_FILE
    stream<mapped_file_source> file_;
#else
    stream<file_source> file_;
#endif
      char* record_;
public:
      dbf_file();
      dbf_file(const std::string& file_name);
      ~dbf_file();
      bool is_open();
      void close();
      int num_records() const;
      int num_fields() const;
      field_descriptor const& descriptor(int col) const;
      void move_to(int index);
      std::string string_value(int col) const;
      void add_attribute(int col, transcoder const& tr, Feature const& f) const throw();
   private:
      dbf_file(const dbf_file&);
      dbf_file& operator=(const dbf_file&);
      void read_header();
      int read_short();
      int read_int();
      void skip(int bytes);
};

#endif //DBFFILE_HPP
