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

#ifndef SHAPEFILE_HPP
#define SHAPEFILE_HPP

// stl
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <cstdint>

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/util/utf_conv_win.hpp>
#include <mapnik/box2d.hpp>

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#pragma GCC diagnostic pop
#include <mapnik/mapped_memory_cache.hpp>
#endif
#include <mapnik/util/noncopyable.hpp>

using mapnik::box2d;
using mapnik::read_int32_ndr;
using mapnik::read_int32_xdr;
using mapnik::read_double_ndr;
using mapnik::read_double_xdr;


struct RecordTag
{
    using data_type = char*;
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
    using data_type = const char*;
    static data_type alloc(unsigned) { return 0; }
    static void dealloc(data_type ) {}
};

template <typename Tag>
struct shape_record
{
    typename Tag::data_type data;
    std::size_t size;
    mutable std::size_t pos;

    explicit shape_record(size_t size_)
        : data(Tag::alloc(size_)),
          size(size_),
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

    void set_pos(unsigned pos_)
    {
        pos = pos_;
    }

    int read_ndr_integer()
    {
        std::int32_t val;
        read_int32_ndr(&data[pos], val);
        pos += 4;
        return val;
    }

    int read_xdr_integer()
    {
        std::int32_t val;
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

    std::size_t length() {return size;}
};

class shape_file : mapnik::util::noncopyable
{
public:

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    using file_source_type = boost::interprocess::ibufferstream;
    using record_type = shape_record<MappedRecordTag>;
    mapnik::mapped_region_ptr mapped_region_;
#else
    using file_source_type = std::ifstream;
    using record_type = shape_record<RecordTag>;
#endif

    file_source_type file_;

    shape_file() {}

    shape_file(std::string  const& file_name) :
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
        file_()
#elif defined (_WINDOWS)
        file_(mapnik::utf8_to_utf16(file_name), std::ios::in | std::ios::binary)
#else
        file_(file_name.c_str(), std::ios::in | std::ios::binary)
#endif
    {
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
        boost::optional<mapnik::mapped_region_ptr> memory =
            mapnik::mapped_memory_cache::instance().find(file_name,true);

        if (memory)
        {
            mapped_region_ = *memory;
            file_.buffer(static_cast<char*>(mapped_region_->get_address()),mapped_region_->get_size());
        }
        else
        {
            throw std::runtime_error("could not create file mapping for "+file_name);
        }
#endif
    }

    ~shape_file() {}

    inline file_source_type& file()
    {
        return file_;
    }

    inline bool is_open() const
    {
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
        return (file_.buffer().second > 0);
#else
        return file_.is_open();
#endif
    }

    inline void read_record(record_type& rec)
    {
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
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
        std::int32_t val;
        read_int32_xdr(b, val);
        return val;
    }

    inline int read_ndr_integer()
    {
        char b[4];
        file_.read(b, 4);
        std::int32_t val;
        read_int32_ndr(b, val);
        return val;
    }

    inline double read_double()
    {
        double val;
        file_.read(reinterpret_cast<char*>(&val), 8);
        return val;
    }

    inline void read_envelope(box2d<double>& envelope)
    {
        file_.read(reinterpret_cast<char*>(&envelope), sizeof(envelope));
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

    inline bool is_good()
    {
        return file_.good();
    }
};

#endif // SHAPEFILE_HPP
