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

#ifndef MAPNIK_GEOMETRY_TO_WKB_HPP
#define MAPNIK_GEOMETRY_TO_WKB_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/make_unique.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_container.hpp>
#include <mapnik/vertex.hpp>

// stl
#include <sstream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <memory>
#include <cassert>

namespace mapnik { namespace util {

std::string to_hex(const char* blob, unsigned size)
{
    std::string buf;
    buf.reserve(size*2);
    std::ostringstream s(buf);
    s.seekp(0);
    char hex[3];
    std::memset(hex,0,3);
    for ( unsigned pos=0; pos < size; ++pos)
    {
        std::sprintf (hex, "%02x", int(blob[pos]) & 0xff);
        s << hex;
    }
    return s.str();
}

enum wkbByteOrder {
    wkbXDR=0,
    wkbNDR=1
};


inline void reverse_bytes(char size, char *address)
{
    char * first = address;
    char * last = first + size - 1;
    for(;first < last;++first, --last)
    {
        char x = *last;
        *last = *first;
        *first = x;
    }
}

struct wkb_stream
{
    wkb_stream(char * buffer, std::size_t size)
        : buffer_(buffer),
          size_(size),
          pos_(0) {}

    void write(char const* data, std::size_t size)
    {
        std::copy(data, data + size, buffer_ + pos_);
        pos_ += size;
    }

    bool good()
    {
        return (pos_ <= size_) ? true : false;
    }

    char * buffer_;
    std::streamsize size_;
    std::streamsize pos_;
};

template <typename S, typename T>
inline void write (S & stream, T val, std::size_t size, wkbByteOrder byte_order)
{
    bool need_swap =  byte_order ? wkbXDR : wkbNDR;
    char* buf = reinterpret_cast<char*>(&val);
    if (need_swap)
    {
        reverse_bytes(size,buf);
    }
    stream.write(buf,size);
}

struct wkb_buffer
{
    wkb_buffer(std::size_t size)
        : size_(size),
          data_( (size_!=0) ? static_cast<char*>(::operator new (size_)):0)
    {}

    ~wkb_buffer()
    {
        ::operator delete(data_);
    }

    inline std::size_t size() const
    {
        return size_;
    }

    inline char* buffer()
    {
        return data_;
    }

    std::size_t size_;
    char * data_;
};

using wkb_buffer_ptr = std::unique_ptr<wkb_buffer>;

template<typename GeometryType>
wkb_buffer_ptr to_point_wkb( GeometryType const& g, wkbByteOrder byte_order)
{
    assert(g.size() == 1);
    std::size_t size = 1 + 4 + 8*2 ; // byteOrder + wkbType + Point
    wkb_buffer_ptr wkb = std::make_unique<wkb_buffer>(size);
    wkb_stream ss(wkb->buffer(), wkb->size());
    ss.write(reinterpret_cast<char*>(&byte_order),1);
    int type = static_cast<int>(mapnik::geometry_type::types::Point);
    write(ss,type,4,byte_order);
    double x = 0;
    double y = 0;
    g.vertex(0,&x,&y);
    write(ss,x,8,byte_order);
    write(ss,y,8,byte_order);
    assert(ss.good());
    return std::move(wkb);
}

template<typename GeometryType>
wkb_buffer_ptr to_line_string_wkb( GeometryType const& g, wkbByteOrder byte_order)
{
    unsigned num_points = g.size();
    assert(num_points > 1);
    std::size_t size = 1 + 4 + 4 + 8*2*num_points ; // byteOrder + wkbType + numPoints + Point*numPoints
    wkb_buffer_ptr wkb = std::make_unique<wkb_buffer>(size);
    wkb_stream ss(wkb->buffer(), wkb->size());
    ss.write(reinterpret_cast<char*>(&byte_order),1);
    int type = static_cast<int>(mapnik::geometry_type::types::LineString);
    write(ss,type,4,byte_order);
    write(ss,num_points,4,byte_order);
    double x = 0;
    double y = 0;
    for (unsigned i=0; i< num_points; ++i)
    {
        g.vertex(i,&x,&y);
        write(ss,x,8,byte_order);
        write(ss,y,8,byte_order);
    }
    assert(ss.good());
    return std::move(wkb);
}

template<typename GeometryType>
wkb_buffer_ptr to_polygon_wkb( GeometryType const& g, wkbByteOrder byte_order)
{
    unsigned num_points = g.size();
    assert(num_points > 1);

    using point_type = std::pair<double,double>;
    using linear_ring = std::vector<point_type>;
    std::vector<linear_ring> rings;

    double x = 0;
    double y = 0;
    std::size_t size = 1 + 4 + 4 ; // byteOrder + wkbType + numRings
    for (unsigned i=0; i< num_points; ++i)
    {
        unsigned command = g.vertex(i,&x,&y);
        if (command == SEG_MOVETO)
        {
            linear_ring ring;
            ring.reserve(1);
            ring.emplace_back(x,y);
            rings.push_back(std::move(ring)); // start new loop
            size += 4; // num_points
            size += 2 * 8; // point
        }
        else if (command == SEG_LINETO)
        {
            rings.back().emplace_back(x,y);
            size += 2 * 8; // point
        }
    }
    unsigned num_rings = rings.size();
    wkb_buffer_ptr wkb = std::make_unique<wkb_buffer>(size);
    wkb_stream ss(wkb->buffer(), wkb->size());
    ss.write(reinterpret_cast<char*>(&byte_order),1);
    int type = static_cast<int>(mapnik::geometry_type::types::Polygon);
    write(ss,type,4,byte_order);
    write(ss,num_rings,4,byte_order);

    for ( linear_ring const& ring : rings)
    {
        unsigned num_ring_points = ring.size();
        write(ss,num_ring_points,4,byte_order);
        for ( point_type const& pt : ring)
        {
            write(ss,pt.first,8,byte_order);
            write(ss,pt.second,8,byte_order);
        }
    }

    assert(ss.good());
    return std::move(wkb);
}

template<typename GeometryType>
wkb_buffer_ptr to_wkb(GeometryType const& g, wkbByteOrder byte_order )
{
    wkb_buffer_ptr wkb;

    switch (g.type())
    {
    case mapnik::geometry_type::types::Point:
        wkb = to_point_wkb(g, byte_order);
        break;
    case mapnik::geometry_type::types::LineString:
        wkb = to_line_string_wkb(g, byte_order);
        break;
    case mapnik::geometry_type::types::Polygon:
        wkb = to_polygon_wkb(g, byte_order);
        break;
    default:
        break;
    }
    return wkb;
}

wkb_buffer_ptr to_wkb(geometry_container const& paths, wkbByteOrder byte_order )
{
    if (paths.size() == 1)
    {
        // single geometry
        return to_wkb(paths.front(), byte_order);
    }

    if (paths.size() > 1)
    {
        // multi geometry or geometry collection
        std::vector<wkb_buffer_ptr> wkb_cont;
        bool collection = false;
        int multi_type = 0;
        size_t multi_size = 1 + 4 + 4;

        for (auto const& geom : paths)
        {
            wkb_buffer_ptr wkb = to_wkb(geom,byte_order);
            multi_size += wkb->size();
            int type = static_cast<int>(geom.type());
            if (multi_type > 0 && multi_type != geom.type())
                collection = true;
            multi_type = type;
            wkb_cont.push_back(std::move(wkb));
        }

        wkb_buffer_ptr multi_wkb = std::make_unique<wkb_buffer>(multi_size);
        wkb_stream ss(multi_wkb->buffer(), multi_wkb->size());
        ss.write(reinterpret_cast<char*>(&byte_order),1);
        multi_type = collection ? 7 : multi_type + 3;
        write(ss,multi_type, 4, byte_order);
        write(ss,paths.size(),4,byte_order);

        for ( wkb_buffer_ptr const& wkb : wkb_cont)
        {
            ss.write(wkb->buffer(),wkb->size());
        }
        return std::move(multi_wkb);
    }

    return wkb_buffer_ptr();
}
}}

#endif // MAPNIK_GEOMETRY_TO_WKB_HPP
