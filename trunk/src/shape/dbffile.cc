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

#include "dbffile.hh"

#include <string>

dbf_file::dbf_file()
    : num_records_(0),
      num_fields_(0),
      record_length_(0),
      record_(0) {}

dbf_file::dbf_file(const char* file_name)
    :num_records_(0),
     num_fields_(0),
     record_length_(0),
     record_(0)
{
    file_.open(file_name);
    if (file_.is_open())
    {
        read_header();
    }
}


dbf_file::~dbf_file()
{
    delete [] record_;
    file_.close();
}


bool dbf_file::open(const std::string& file_name)
{
    file_.open(file_name.c_str());
    if (file_.is_open())
        read_header();
    return file_?true:false;
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


void dbf_file::add_attribute(int col,Feature* f) const
{
    if (col>=0 && col<num_fields_)
    {
        std::string name=fields_[col].name_;
        std::string str(record_+fields_[col].offset_,fields_[col].length_);
        switch (fields_[col].type_)
        {
	case 'C':
	case 'D':                             //todo handle date?
	case 'M':
	    f->add(name,attribute<std::string>(str));
	    break;
	case 'N':
	    f->add(name,attribute_from_string<int>(str));
	    break;
	case 'F':
	    f->add(name,attribute_from_string<double>(str));
	    break;
        }
    }
}


void dbf_file::read_header()
{
    char c=file_.get();
    if (c=='\3' || c=='\131')
    {
        skip(3);
        read_int(num_records_);
        assert(num_records_>0);
        read_short(num_fields_);
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
            desc.name_=name;                      //TODO trim ws
            skip(1);
            desc.type_=file_.get();
            skip(4);
            desc.length_=file_.get();
            desc.dec_=file_.get();
            skip(14);
            desc.offset_=offset;
            offset+=desc.length_;
            //std::cout <<"name="<<desc.name_<<std::endl;
            fields_.push_back(desc);
        }
        record_length_=offset;
        if (record_length_>0)
        {
            record_=new char[record_length_];
        }
    }
}


void dbf_file::read_short(int& n)
{
    char b[2];
    file_.read(b,2);
    memcpy(&n,b,2);
}


void dbf_file::read_int(int& n)
{
    char b[4];
    file_.read(b,4);
    memcpy(&n,b,4);
}


void dbf_file::read_double(double& d)
{
    char b[8];
    file_.read(b,8);
    memcpy(&d,b,8);
}


void dbf_file::skip(int bytes)
{
    file_.seekg(bytes,std::ios::cur);
}
