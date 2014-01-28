/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/value_types.hpp>
#include <mapnik/global.hpp>
#include <mapnik/utils.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/util/trim.hpp>

#include "dbfile.hpp"

// boost
#include <boost/spirit/include/qi.hpp>
#include <boost/cstdint.hpp> // for int16_t and int32_t
#ifdef SHAPE_MEMORY_MAPPED_FILE
#include <boost/interprocess/mapped_region.hpp>
#include <mapnik/mapped_memory_cache.hpp>
#endif

// stl
#include <string>
#include <cstring>
#include <stdexcept>

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
     file_(),
#elif defined(_WINDOWS)
     file_(mapnik::utf8_to_utf16(file_name), std::ios::in | std::ios::binary),
#else
     file_(file_name.c_str() ,std::ios::in | std::ios::binary),
#endif
     record_(0)
{

#ifdef SHAPE_MEMORY_MAPPED_FILE
    boost::optional<mapnik::mapped_region_ptr> memory = mapnik::mapped_memory_cache::instance().find(file_name,true);
    if (memory)
    {
        mapped_region_ = *memory;
        file_.buffer(static_cast<char*>((*memory)->get_address()),(*memory)->get_size());
    }
    else
    {
        throw std::runtime_error("could not create file mapping for "+file_name);
    }
#endif
    if (file_)
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
#ifdef SHAPE_MEMORY_MAPPED_FILE
    return (file_.buffer().second > 0);
#else
    return file_.is_open();
#endif
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
        std::streampos pos=(num_fields_<<5)+34+(index-1)*(record_length_+1);
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


void dbf_file::add_attribute(int col, mapnik::transcoder const& tr, mapnik::feature_impl & f) const throw()
{
    using namespace boost::spirit;

    if (col>=0 && col<num_fields_)
    {
        std::string const& name=fields_[col].name_;

        // NOTE: ensure types handled here are matched in shape_datasource.cpp
        switch (fields_[col].type_)
        {
        case 'C':
        case 'D':
        {
            // FIXME - avoid constructing std::string on stack
            std::string str(record_+fields_[col].offset_,fields_[col].length_);
            mapnik::util::trim(str);
            f.put(name,tr.transcode(str.c_str()));
            break;
        }
        case 'L':
        {
            char ch = record_[fields_[col].offset_];
            if ( ch == '1' || ch == 't' || ch == 'T' || ch == 'y' || ch == 'Y')
            {
                f.put(name,true);
            }
            else
            {
                // NOTE: null logical fields use '?'
                f.put(name,false);
            }
            break;
        }
        case 'N': // numeric
        case 'O': // double
        case 'F': // float
        {

            if (record_[fields_[col].offset_] == '*')
            {
                // NOTE: we intentionally do not store null here
                // since it is equivalent to the attribute not existing
                break;
            }
            if ( fields_[col].dec_>0 )
            {
                double val = 0.0;
                const char *itr = record_+fields_[col].offset_;
                const char *end = itr + fields_[col].length_;
                ascii::space_type space;
                qi::double_type double_;
                if (qi::phrase_parse(itr,end,double_,space,val))
                {
                    f.put(name,val);
                }
            }
            else
            {
                mapnik::value_integer val = 0;
                const char *itr = record_+fields_[col].offset_;
                const char *end = itr + fields_[col].length_;
                ascii::space_type space;
                qi::int_type int_;
                if (qi::phrase_parse(itr,end,int_,space,val))
                {
                    f.put(name,val);
                }
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
        std::streampos offset=0;
        char name[11];
        std::memset(&name,0,11);
        fields_.reserve(num_fields_);
        for (int i=0;i<num_fields_;++i)
        {
            field_descriptor desc;
            desc.index_=i;
            file_.read(name,10);
            desc.name_=name;
            // TODO - when is this trim needed?
            mapnik::util::trim(desc.name_);
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
