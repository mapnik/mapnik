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

// mapnik
#include <mapnik/make_unique.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/global.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/geometry/correct.hpp>

#include <memory>

namespace mapnik
{

struct wkb_reader : util::noncopyable
{
private:
    const char* wkb_;
    std::size_t size_;
    std::size_t pos_;
    wkbByteOrder byteOrder_;
    bool needSwap_;
    wkbFormat format_;

public:

    enum wkbGeometryType {
        wkbPoint=1,
        wkbLineString=2,
        wkbPolygon=3,
        wkbMultiPoint=4,
        wkbMultiLineString=5,
        wkbMultiPolygon=6,
        wkbGeometryCollection=7,
        // Z
        wkbPointZ=1001,
        wkbLineStringZ=1002,
        wkbPolygonZ=1003,
        wkbMultiPointZ=1004,
        wkbMultiLineStringZ=1005,
        wkbMultiPolygonZ=1006,
        wkbGeometryCollectionZ=1007,
        // M
        wkbPointM=2001,
        wkbLineStringM=2002,
        wkbPolygonM=2003,
        wkbMultiPointM=2004,
        wkbMultiLineStringM=2005,
        wkbMultiPolygonM=2006,
        wkbGeometryCollectionM=2007,
        // ZM
        wkbPointZM=3001,
        wkbLineStringZM=3002,
        wkbPolygonZM=3003,
        wkbMultiPointZM=3004,
        wkbMultiLineStringZM=3005,
        wkbMultiPolygonZM=3006,
        wkbGeometryCollectionZM=3007
    };

    wkb_reader(const char* wkb, std::size_t size, wkbFormat format)
        : wkb_(wkb),
          size_(size),
          pos_(0),
          format_(format)
    {
        // try to determine WKB format automatically
        if (format_ == wkbAuto)
        {
            if (size_ >= 44
                && static_cast<unsigned char>(wkb_[0]) == static_cast<unsigned char>(0x00)
                && static_cast<unsigned char>(wkb_[38]) == static_cast<unsigned char>(0x7C)
                && static_cast<unsigned char>(wkb_[size_ - 1]) == static_cast<unsigned char>(0xFE))
            {
                format_ = wkbSpatiaLite;
            }
            else
            {
                format_ = wkbGeneric;
            }
        }

        switch (format_)
        {
        case wkbSpatiaLite:
            byteOrder_ = static_cast<wkbByteOrder>(wkb_[1]);
            pos_ = 39;
            break;

        case wkbGeneric:
        default:
            byteOrder_ = static_cast<wkbByteOrder>(wkb_[0]);
            pos_ = 1;
            break;
        }

        needSwap_ = byteOrder_ ? wkbXDR : wkbNDR;
    }

    mapnik::geometry::geometry<double> read()
    {
        mapnik::geometry::geometry<double> geom = mapnik::geometry::geometry_empty();
        int type = read_integer();
        switch (type)
        {
        case wkbPoint:
        {
            auto pt = read_point();
            if (!std::isnan(pt.x) && !std::isnan(pt.y))
                geom = std::move(pt);
            break;
        }
        case wkbLineString:
            geom = read_linestring();
            break;
        case wkbPolygon:
            geom = read_polygon();
            break;
        case wkbMultiPoint:
            geom = read_multipoint();
            break;
        case wkbMultiLineString:
            geom = read_multilinestring();
            break;
        case wkbMultiPolygon:
            geom = read_multipolygon();
            break;
        case wkbGeometryCollection:
            geom = read_collection();
            break;
        case wkbPointZ:
        case wkbPointM:
        {
            auto pt = read_point<true>();
            if (!std::isnan(pt.x) && !std::isnan(pt.y))
                geom = std::move(pt);
            break;
        }
        case wkbPointZM:
        {
            auto pt = read_point<true,true>();
            if (!std::isnan(pt.x) && !std::isnan(pt.y))
                geom = std::move(pt);
            break;
        }
        case wkbLineStringZ:
        case wkbLineStringM:
            geom = read_linestring<true>();
            break;
        case wkbLineStringZM:
            geom = read_linestring<true,true>();
            break;
        case wkbPolygonZ:
        case wkbPolygonM:
            geom = read_polygon<true>();
            break;
        case wkbPolygonZM:
            geom = read_polygon<true,true>();
            break;
        case wkbMultiPointZ:
        case wkbMultiPointM:
            geom = read_multipoint<true>();
            break;
        case wkbMultiPointZM:
            geom = read_multipoint<true,true>();
            break;
        case wkbMultiLineStringZ:
        case wkbMultiLineStringM:
            geom = read_multilinestring<true>();
            break;
        case wkbMultiLineStringZM:
            geom = read_multilinestring<true,true>();
            break;
        case wkbMultiPolygonZ:
        case wkbMultiPolygonM:
            geom = read_multipolygon<true>();
            break;
        case wkbMultiPolygonZM:
            geom = read_multipolygon<true,true>();
            break;
        case wkbGeometryCollectionZ:
        case wkbGeometryCollectionM:
        case wkbGeometryCollectionZM:
            geom = read_collection();
            break;
        default:
            break;
        }
        return geom;
    }

private:

    int read_integer()
    {
        std::int32_t n;
        if (needSwap_)
        {
            read_int32_xdr(wkb_ + pos_, n);
        }
        else
        {
            read_int32_ndr(wkb_ + pos_, n);
        }
        pos_ += 4;

        return n;
    }

    double read_double()
    {
        double d;
        if (needSwap_)
        {
            read_double_xdr(wkb_ + pos_, d);
        }
        else
        {
            read_double_ndr(wkb_ + pos_, d);
        }
        pos_ += 8;

        return d;
    }

    template <typename Ring, bool Z = false, bool M = false>
    void read_coords(Ring & ring, std::size_t num_points)
    {
        double x,y;
        if (!needSwap_)
        {
            for (std::size_t i = 0; i < num_points; ++i)
            {
                read_double_ndr(wkb_ + pos_, x);
                read_double_ndr(wkb_ + pos_ + 8, y);
                ring.emplace_back(x,y);
                pos_ += 16; // skip XY
                if (Z) pos_ += 8;
                if (M) pos_ += 8;
            }
        }
        else
        {
            for (std::size_t i = 0; i < num_points; ++i)
            {
                read_double_xdr(wkb_ + pos_, x);
                read_double_xdr(wkb_ + pos_ + 8, y);
                ring.emplace_back(x,y);
                pos_ += 16; // skip XY
                if (Z) pos_ += 8;
                if (M) pos_ += 8;
            }
        }
    }

    template <bool Z = false, bool M = false>
    mapnik::geometry::point<double> read_point()
    {
        double x = read_double();
        double y = read_double();
        if (Z) pos_ += 8;
        if (M) pos_ += 8;
        return mapnik::geometry::point<double>(x, y);
    }

    template <bool Z = false, bool M = false>
    mapnik::geometry::multi_point<double> read_multipoint()
    {
        mapnik::geometry::multi_point<double> multi_point;
        int num_points = read_integer();
        multi_point.reserve(num_points);
        for (int i = 0; i < num_points; ++i)
        {
            pos_ += 5;
            multi_point.emplace_back(read_point<Z,M>());
        }
        return multi_point;
    }

    template <bool M = false, bool Z = false>
    mapnik::geometry::line_string<double> read_linestring()
    {
        mapnik::geometry::line_string<double> line;
        int num_points = read_integer();
        if (num_points > 0)
        {
            line.reserve(num_points);
            read_coords<mapnik::geometry::line_string<double>, M, Z>(line, num_points);
        }
        return line;
    }

    template <bool M = false, bool Z = false>
    mapnik::geometry::multi_line_string<double> read_multilinestring()
    {
        int num_lines = read_integer();
        mapnik::geometry::multi_line_string<double> multi_line;
        multi_line.reserve(num_lines);
        for (int i = 0; i < num_lines; ++i)
        {
            pos_ += 5;
            multi_line.push_back(read_linestring<M, Z>());
        }
        return multi_line;
    }

    template <bool M = false, bool Z = false>
    mapnik::geometry::polygon<double> read_polygon()
    {
        int num_rings = read_integer();
        mapnik::geometry::polygon<double> poly;
        poly.reserve(num_rings);
        for (int i = 0; i < num_rings; ++i)
        {
            mapnik::geometry::linear_ring<double> ring;
            int num_points = read_integer();
            if (num_points > 0)
            {
                ring.reserve(num_points);
                read_coords<mapnik::geometry::linear_ring<double>, M, Z>(ring, num_points);
            }
            poly.push_back(std::move(ring));
        }
        return poly;
    }

    template <bool M = false, bool Z = false>
    mapnik::geometry::multi_polygon<double> read_multipolygon()
    {
        int num_polys = read_integer();
        mapnik::geometry::multi_polygon<double> multi_poly;
        multi_poly.reserve(num_polys);
        for (int i = 0; i < num_polys; ++i)
        {
            pos_ += 5;
            multi_poly.push_back(read_polygon<M, Z>());
        }
        return multi_poly;
    }

    mapnik::geometry::geometry_collection<double> read_collection()
    {
        int num_geometries = read_integer();
        mapnik::geometry::geometry_collection<double> collection;
        collection.reserve(num_geometries);
        for (int i = 0; i < num_geometries; ++i)
        {
            pos_ += 1; // skip byte order
            collection.push_back(read());
         }
        return collection;
     }

    std::string wkb_geometry_type_string(int type)
    {
        std::stringstream s;

        switch (type)
        {
        case wkbPoint:               s << "Point"; break;
        case wkbPointZ:              s << "PointZ"; break;
        case wkbPointM:              s << "PointM"; break;
        case wkbPointZM:             s << "PointZM"; break;
        case wkbMultiPoint:          s << "MultiPoint"; break;
        case wkbMultiPointZ:         s << "MultiPointZ"; break;
        case wkbMultiPointM:         s << "MultiPointM"; break;
        case wkbMultiPointZM:        s << "MultiPointZM"; break;
        case wkbLineString:          s << "LineString"; break;
        case wkbLineStringZ:         s << "LineStringZ"; break;
        case wkbLineStringM:         s << "LineStringM"; break;
        case wkbLineStringZM:        s << "LineStringZM"; break;
        case wkbMultiLineString:     s << "MultiLineString"; break;
        case wkbMultiLineStringZ:    s << "MultiLineStringZ"; break;
        case wkbMultiLineStringM:    s << "MultiLineStringM"; break;
        case wkbMultiLineStringZM:   s << "MultiLineStringZM"; break;
        case wkbPolygon:             s << "Polygon"; break;
        case wkbPolygonZ:            s << "PolygonZ"; break;
        case wkbPolygonM:            s << "PolygonM"; break;
        case wkbPolygonZM:           s << "PolygonZM"; break;
        case wkbMultiPolygon:        s << "MultiPolygon"; break;
        case wkbMultiPolygonZ:       s << "MultiPolygonZ"; break;
        case wkbMultiPolygonM:       s << "MultiPolygonM"; break;
        case wkbMultiPolygonZM:      s << "MultiPolygonZM"; break;
        case wkbGeometryCollection:  s << "GeometryCollection"; break;
        case wkbGeometryCollectionZ: s << "GeometryCollectionZ"; break;
        case wkbGeometryCollectionM: s << "GeometryCollectionM"; break;
        case wkbGeometryCollectionZM:s << "GeometryCollectionZM"; break;
        default:                     s << "wkbUnknown(" << type << ")"; break;
        }

        return s.str();
    }

};

mapnik::geometry::geometry<double> geometry_utils::from_wkb(const char* wkb,
                                                            std::size_t size,
                                                            wkbFormat format)
{
    wkb_reader reader(wkb, size, format);
    mapnik::geometry::geometry<double> geom(reader.read());
    // note: this will only be applied to polygons
    mapnik::geometry::correct(geom);
    return geom;
}

} // namespace mapnik
