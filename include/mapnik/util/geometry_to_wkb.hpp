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

#ifndef MAPNIK_GEOMETRY_TO_WKB_HPP
#define MAPNIK_GEOMETRY_TO_WKB_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/make_unique.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_type.hpp>

// stl
#include <sstream>
#include <vector>
#include <cstdio>
#include <cstring>
#include <memory>
#include <cassert>

namespace mapnik { namespace util { namespace detail {

std::string to_hex(const char* blob, std::size_t size)
{
    std::string buf;
    buf.reserve(size * 2);
    std::ostringstream s(buf);
    s.seekp(0);
    char hex[3];
    std::memset(hex, 0, 3);
    for (std::size_t pos = 0; pos < size; ++pos)
    {
        std::sprintf (hex, "%02x", int(blob[pos]) & 0xff);
        s << hex;
    }
    return s.str();
}

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
          data_( (size_ != 0) ? static_cast<char*>(::operator new (size_)) : 0)
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

wkb_buffer_ptr point_wkb( geometry::point<double> const& pt, wkbByteOrder byte_order)
{
    std::size_t size = 1 + 4 + 8 * 2 ; // byteOrder + wkbType + Point
    wkb_buffer_ptr wkb = std::make_unique<wkb_buffer>(size);
    wkb_stream ss(wkb->buffer(), wkb->size());
    ss.write(reinterpret_cast<char*>(&byte_order),1);
    write(ss, static_cast<int>(geometry::geometry_types::Point), 4 , byte_order);
    write(ss, pt.x, 8, byte_order);
    write(ss, pt.y, 8, byte_order);
    assert(ss.good());
    return wkb;
}

wkb_buffer_ptr line_string_wkb(geometry::line_string<double> const& line, wkbByteOrder byte_order)
{
    std::size_t num_points = line.size();
    assert(num_points > 1);
    std::size_t size = 1 + 4 + 4 + 8 * 2 * num_points ; // byteOrder + wkbType + numPoints + Point*numPoints
    wkb_buffer_ptr wkb = std::make_unique<wkb_buffer>(size);
    wkb_stream ss(wkb->buffer(), wkb->size());
    ss.write(reinterpret_cast<char*>(&byte_order),1);
    write(ss, static_cast<int>(mapnik::geometry::geometry_types::LineString) , 4, byte_order);
    write(ss, num_points, 4, byte_order);
    for (std::size_t i=0; i< num_points; ++i)
    {
        geometry::point<double> const& pt = line[i];
        write(ss, pt.x, 8, byte_order);
        write(ss, pt.y, 8, byte_order);
    }
    assert(ss.good());
    return wkb;
}

wkb_buffer_ptr polygon_wkb( geometry::polygon<double> const& poly, wkbByteOrder byte_order)
{
    std::size_t size = 1 + 4 + 4 ; // byteOrder + wkbType + numRings
    size += 4 + 2 * 8 * poly.exterior_ring.size();
    for ( auto const& ring : poly.interior_rings)
    {

        size += 4 + 2 * 8 * ring.size();
    }

    wkb_buffer_ptr wkb = std::make_unique<wkb_buffer>(size);
    wkb_stream ss(wkb->buffer(), wkb->size());
    ss.write(reinterpret_cast<char*>(&byte_order),1);
    write(ss, static_cast<int>(mapnik::geometry::geometry_types::Polygon), 4, byte_order);
    write(ss, poly.num_rings(), 4, byte_order);

    // exterior
    write(ss, poly.exterior_ring.size(), 4, byte_order);
    for (auto const& pt : poly.exterior_ring)
    {
        write(ss, pt.x, 8, byte_order);
        write(ss, pt.y, 8, byte_order);
    }
    // interiors
    for (auto const& ring : poly.interior_rings)
    {
        write(ss, ring.size(), 4, byte_order);
        for ( auto const& pt : ring)
        {
            write(ss, pt.x, 8, byte_order);
            write(ss, pt.y, 8, byte_order);
        }
    }

    assert(ss.good());
    return wkb;
}

wkb_buffer_ptr multi_point_wkb( geometry::multi_point<double> const& multi_pt, wkbByteOrder byte_order)
{
    std::size_t size = 1 + 4 + 4 + (1 + 4 + 8 * 2) * multi_pt.size() ; // byteOrder + wkbType + num_point + Point.size * num_points
    wkb_buffer_ptr wkb = std::make_unique<wkb_buffer>(size);
    wkb_stream ss(wkb->buffer(), wkb->size());
    ss.write(reinterpret_cast<char*>(&byte_order),1);
    write(ss, static_cast<int>(geometry::geometry_types::MultiPoint), 4, byte_order);
    write(ss, multi_pt.size(), 4 ,byte_order);
    for (auto const& pt : multi_pt)
    {
        ss.write(reinterpret_cast<char*>(&byte_order),1);
        write(ss, static_cast<int>(geometry::geometry_types::Point), 4, byte_order);
        write(ss, pt.x, 8, byte_order);
        write(ss, pt.y, 8, byte_order);
    }
    assert(ss.good());
    return wkb;
}


template <typename MultiGeometry>
    wkb_buffer_ptr multi_geom_wkb(MultiGeometry const& multi_geom, wkbByteOrder byte_order);

struct geometry_to_wkb
{

    using result_type = wkb_buffer_ptr;

    geometry_to_wkb(wkbByteOrder byte_order)
        : byte_order_(byte_order) {}

    result_type operator() (geometry::geometry<double> const& geom) const
    {
        return util::apply_visitor(*this, geom);
    }

    result_type operator() (geometry::geometry_empty const&) const
    {
        return result_type();
    }

    result_type operator() (geometry::point<double> const& pt) const
    {
        return point_wkb(pt, byte_order_);
    }

    result_type operator() (geometry::line_string<double> const& line) const
    {
        return line_string_wkb(line, byte_order_);
    }

    result_type operator() (geometry::polygon<double> const& poly) const
    {
        return polygon_wkb(poly, byte_order_);
    }

    // multi/collection

    result_type operator() (geometry::multi_point<double> const& multi_pt) const
    {
        return multi_point_wkb(multi_pt, byte_order_);
    }

    template <typename Geometry>
    result_type operator() (Geometry const& geom) const
    {
        return multi_geom_wkb(geom, byte_order_);
    }

    wkbByteOrder byte_order_;
};

template <typename MultiGeometry>
wkb_buffer_ptr multi_geom_wkb(MultiGeometry const& multi_geom, wkbByteOrder byte_order)
{
    size_t multi_size = 1 + 4 + 4;
    std::vector<wkb_buffer_ptr> wkb_cont;
    for (auto const& geom : multi_geom)
    {
        wkb_buffer_ptr wkb = geometry_to_wkb(byte_order)(geom);
        multi_size += wkb->size();
        wkb_cont.push_back(std::move(wkb));
    }
    wkb_buffer_ptr multi_wkb = std::make_unique<wkb_buffer>(multi_size);
    wkb_stream ss(multi_wkb->buffer(), multi_wkb->size());
    ss.write(reinterpret_cast<char*>(&byte_order),1);
    write(ss, static_cast<int>(geometry::detail::geometry_type()(multi_geom)) , 4, byte_order);
    write(ss, multi_geom.size(), 4 ,byte_order);

    for ( wkb_buffer_ptr const& wkb : wkb_cont)
    {
        ss.write(wkb->buffer(), wkb->size());
    }

    return multi_wkb;
}
} // ns detail

using wkb_buffer_ptr = detail::wkb_buffer_ptr;

template<typename GeometryType>
wkb_buffer_ptr to_wkb(GeometryType const& geom, wkbByteOrder byte_order )
{
    return detail::geometry_to_wkb(byte_order)(geom);
}

}}

#endif // MAPNIK_GEOMETRY_TO_WKB_HPP
