/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/unicode.hpp>

#include <mapnik/util/mapped_memory_file.hpp>

// stl
#include <vector>
#include <string>
#include <cassert>
#include <fstream>

struct field_descriptor
{
    int index_;
    std::string name_;
    char type_;
    int length_;
    int dec_;
    std::streampos offset_;
};


class dbf_file : public mapnik::util::mapped_memory_file
{
private:
    int num_records_;
    int num_fields_;
    std::size_t record_length_;
    std::vector<field_descriptor> fields_;
    char* record_;
public:
    dbf_file();
    dbf_file(std::string const& file_name);
    ~dbf_file();
    int num_records() const;
    int num_fields() const;
    field_descriptor const& descriptor(int col) const;
    void move_to(int index);
    std::string string_value(int col) const;
    void add_attribute(int col, mapnik::transcoder const& tr, mapnik::feature_impl & f) const;
private:
    void read_header();
    int read_short();
    int read_int();
};

#endif //DBFFILE_HPP
