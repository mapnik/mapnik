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

#ifndef SHAPEFILE_HPP
#define SHAPEFILE_HPP

// stl
#include <cstring>
#include <fstream>

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/mapped_memory_cache.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/cstdint.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>

using mapnik::box2d;
using mapnik::read_int32_ndr;
using mapnik::read_int32_xdr;
using mapnik::read_double_ndr;
using mapnik::read_double_xdr;


struct RecordTag
{
    typedef char* data_type;
    static data_type alloc(unsigned size)
    {
        return static_cast<data_type>(::operator new(sizeof(char)*size));
    }

    static void dealloc(data_type data)
    {
        ::operator delete(data);
    }
};

struct MappedRecordTag
{
    typedef const char* data_type;
    static data_type alloc(unsigned) { return 0; }
    static void dealloc(data_type ) {}
};

template <typename Tag>
struct shape_record
{
    typename Tag::data_type data;
    size_t size;
    mutable size_t pos;

    explicit shape_record(size_t size)
        : data(Tag::alloc(size)),
          size(size),
          pos(0)
    {}

    ~shape_record()
    {
        Tag::dealloc(data);
    }

    void set_data(typename Tag::data_type data_)
    {
        data = data_;
    }

    typename Tag::data_type get_data()
    {
        return data;
    }

    void skip(unsigned n)
    {
        pos += n;
    }

    int read_ndr_integer()
    {
        boost::int32_t val;
        read_int32_ndr(&data[pos], val);
        pos += 4;
        return val;
    }

    int read_xdr_integer()
    {
        boost::int32_t val;
        read_int32_xdr(&data[pos], val);
        pos += 4;
        return val;
    }

    double read_double()
    {
        double val;
        read_double_ndr(&data[pos], val);
        pos += 8;
        return val;
    }

    long remains()
    {
        return (size - pos);
    }
};

using namespace boost::interprocess;

class shape_file : boost::noncopyable
{
public:

#ifdef SHAPE_MEMORY_MAPPED_FILE
    typedef ibufferstream file_source_type;
    typedef shape_record<MappedRecordTag> record_type;
#else
    typedef std::ifstream file_source_type;
    typedef shape_record<RecordTag> record_type;
#endif

    file_source_type file_;

    shape_file() {}

    shape_file(std::string  const& file_name) :
#ifdef SHAPE_MEMORY_MAPPED_FILE
        file_()
#else
        file_(file_name.c_str(), std::ios::in | std::ios::binary)
#endif
    {
#ifdef SHAPE_MEMORY_MAPPED_FILE
        boost::optional<mapnik::mapped_region_ptr> memory =
            mapnik::mapped_memory_cache::instance().find(file_name.c_str(),true);

        if (memory)
        {
            file_.buffer(static_cast<char*>((*memory)->get_address()), (*memory)->get_size());
        }
#endif
    }

    ~shape_file() {}

    inline file_source_type& file()
    {
        return file_;
    }

    inline bool is_open()
    {
#ifdef SHAPE_MEMORY_MAPPED_FILE
        return (file_.buffer().second > 0);
#else
        return file_.is_open();
#endif
    }

    inline void read_record(record_type& rec)
    {
#ifdef SHAPE_MEMORY_MAPPED_FILE
        rec.set_data(file_.buffer().first + file_.tellg());
        file_.seekg(rec.size, std::ios::cur);
#else
        file_.read(rec.get_data(), rec.size);
#endif
    }

    inline int read_xdr_integer()
    {
        char b[4];
        file_.read(b, 4);
        boost::int32_t val;
        read_int32_xdr(b, val);
        return val;
    }

    inline int read_ndr_integer()
    {
        char b[4];
        file_.read(b, 4);
        boost::int32_t val;
        read_int32_ndr(b, val);
        return val;
    }

    inline double read_double()
    {
        double val;
#ifndef MAPNIK_BIG_ENDIAN
        file_.read(reinterpret_cast<char*>(&val), 8);
#else
        char b[8];
        file_.read(b, 8);
        read_double_ndr(b, val);
#endif
        return val;
    }

    inline void read_envelope(box2d<double>& envelope)
    {
#ifndef MAPNIK_BIG_ENDIAN
        file_.read(reinterpret_cast<char*>(&envelope), sizeof(envelope));
#else
        char data[4 * 8];
        file_.read(data,4 * 8);
        double minx, miny, maxx, maxy;
        read_double_ndr(data + 0 * 8, minx);
        read_double_ndr(data + 1 * 8, miny);
        read_double_ndr(data + 2 * 8, maxx);
        read_double_ndr(data + 3 * 8, maxy);
        envelope.init(minx, miny, maxx, maxy);
#endif
    }

    inline void skip(std::streampos bytes)
    {
        file_.seekg(bytes, std::ios::cur);
    }

    inline void rewind()
    {
        seek(100);
    }

    inline void seek(std::streampos pos)
    {
        file_.seekg(pos, std::ios::beg);
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

#endif // SHAPEFILE_HPP
