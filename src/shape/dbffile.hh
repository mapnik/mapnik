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

#ifndef DBFFILE_HH
#define DBFFILE_HH

#include <vector>
#include <string>
#include <fstream>
#include <cassert>
//#include <iostream>


#include "mapnik.hh"

using namespace mapnik;

struct field_descriptor
{
    public:
        int index_;
        std::string name_;
        char type_;
        int length_;
        int dec_;
        int offset_;
};

class dbf_file
{
    private:
        enum
        {
            String=1,
            Numeric,
            Date,
            Float,
            Other
        };
        int num_records_;
        int num_fields_;
        int record_length_;
        std::vector<field_descriptor> fields_;
        std::ifstream file_;
        char* record_;
    public:
        dbf_file();
        dbf_file(const char* file_name);
        dbf_file(const std::string& file_name);
        ~dbf_file();
        bool open(const std::string& file_name);
        bool is_open();
        void close();
        int num_records() const;
        int num_fields() const;
        const field_descriptor& descriptor(int col) const;
        void move_to(int index);
        std::string string_value(int col) const;
        void add_attribute(int col,Feature* f) const;
    private:
        dbf_file(const dbf_file&);
        dbf_file& operator=(const dbf_file&);
        void read_header();
        void read_short(int& n);
        void read_int(int& n);
        void read_double(double& d);
        void skip(int bytes);
};
#endif                                            //DBFFILE_HH
