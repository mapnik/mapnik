/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/coord_array.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/util/noncopyable.hpp>

namespace mapnik
{

using CoordinateArray = coord_array<coord2d>;

struct wkb_reader : util::noncopyable
{
private:
    enum wkbByteOrder
    {
        wkbXDR=0,
        wkbNDR=1
    };

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
            byteOrder_ = (wkbByteOrder) wkb_[1];
            pos_ = 39;
            break;

        case wkbGeneric:
        default:
            byteOrder_ = (wkbByteOrder) wkb_[0];
            pos_ = 1;
            break;
        }

        needSwap_ = byteOrder_ ? wkbXDR : wkbNDR;
    }

    mapnik::new_geometry::geometry read()
    {
        int type = read_integer();
        switch (type)
        {
        case wkbPoint:
            return std::move(mapnik::new_geometry::geometry(read_point()));
        case wkbLineString:
            return std::move(mapnik::new_geometry::geometry(read_linestring()));
        case wkbPolygon:
            return std::move(mapnik::new_geometry::geometry(read_polygon()));
        case wkbMultiPoint:
            return std::move(mapnik::new_geometry::geometry(read_multipoint()));
        case wkbMultiLineString:
            return std::move(mapnik::new_geometry::geometry(read_multilinestring()));
        case wkbMultiPolygon:
            return std::move(mapnik::new_geometry::geometry(read_multipolygon()));
        case wkbGeometryCollection:
            throw std::runtime_error("GeometryCollection");
            break; // TODO: should we drop geometry collection ?
        case wkbPointZ:
        case wkbPointM:
            return std::move(mapnik::new_geometry::geometry(read_point<true>()));
        case wkbPointZM:
            return std::move(mapnik::new_geometry::geometry(read_point<true,true>()));
        case wkbLineStringZ:
        case wkbLineStringM:
            return std::move(mapnik::new_geometry::geometry(read_linestring<true>()));
        case wkbLineStringZM:
            return std::move(mapnik::new_geometry::geometry(read_linestring<true,true>()));
        case wkbPolygonZ:
        case wkbPolygonM:
            return std::move(mapnik::new_geometry::geometry(read_polygon<true>()));
        case wkbPolygonZM:
            return std::move(mapnik::new_geometry::geometry(read_polygon<true,true>()));
        case wkbMultiPointZ:
        case wkbMultiPointM:
            return std::move(mapnik::new_geometry::geometry(read_multipoint<true>()));
        case wkbMultiPointZM:
            return std::move(mapnik::new_geometry::geometry(read_multipoint<true,true>()));
        case wkbMultiLineStringZ:
        case wkbMultiLineStringM:
            return std::move(mapnik::new_geometry::geometry(read_multilinestring<true>()));
        case wkbMultiLineStringZM:
            return std::move(mapnik::new_geometry::geometry(read_multilinestring<true,true>()));
        case wkbMultiPolygonZ:
        case wkbMultiPolygonM:
            return std::move(mapnik::new_geometry::geometry(read_multipolygon<true>()));
        case wkbMultiPolygonZM:
            return std::move(mapnik::new_geometry::geometry(read_multipolygon<true,true>()));
        case wkbGeometryCollectionZ:
        case wkbGeometryCollectionM:
        case wkbGeometryCollectionZM:
            // TODO ??
            throw std::runtime_error("GeometryCollection Z|M|ZM");
            break;
        default:
            break;
        }
        throw std::runtime_error("Uknown");
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

    template <bool Z = false, bool M = false>
    void read_coords(CoordinateArray& ar)
    {
        if (! needSwap_)
        {
            for (auto & coord : ar)
            {
                read_double_ndr(wkb_ + pos_, coord.x);
                read_double_ndr(wkb_ + pos_ + 8, coord.y);
                pos_ += 16; // skip XY
                if (Z) pos_ += 8;
                if (M) pos_ += 8;
            }
        }
        else
        {
            for (auto & coord : ar)
            {
                read_double_xdr(wkb_ + pos_, coord.x);
                read_double_xdr(wkb_ + pos_ + 8, coord.y);
                pos_ += 16; // skip XY
                if (Z) pos_ += 8;
                if (M) pos_ += 8;
            }
        }
    }

    template <bool Z = false, bool M = false>
    mapnik::new_geometry::point read_point()
    {
        double x = read_double();
        double y = read_double();
        if (Z) pos_ += 8;
        if (M) pos_ += 8;
        return mapnik::new_geometry::point(x, y);
    }

    template <bool Z = false, bool M = false>
    mapnik::new_geometry::multi_point read_multipoint()
    {
        mapnik::new_geometry::multi_point multi_point;
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
    mapnik::new_geometry::line_string read_linestring()
    {
        mapnik::new_geometry::line_string line;
        int num_points = read_integer();
        if (num_points > 0)
        {
            CoordinateArray ar(num_points);
            read_coords<M, Z>(ar);

            line.reserve(num_points);
            for (int i = 0; i < num_points; ++i)
            {
                line.add_coord(ar[i].x, ar[i].y);
            }
        }
        return line;
    }

    template <bool M = false, bool Z = false>
    mapnik::new_geometry::multi_line_string read_multilinestring()
    {
        int num_lines = read_integer();
        mapnik::new_geometry::multi_line_string multi_line;
        multi_line.reserve(num_lines);
        for (int i = 0; i < num_lines; ++i)
        {
            pos_ += 5;
            multi_line.push_back(std::move(read_linestring<M, Z>()));
        }
        return multi_line;
    }

    template <bool M = false, bool Z = false>
    mapnik::new_geometry::polygon3 read_polygon()
    {
        int num_rings = read_integer();
        mapnik::new_geometry::polygon3 poly;
        if (num_rings > 1)
        {
            poly.interior_rings.reserve(num_rings - 1);
        }

        for (int i = 0; i < num_rings; ++i)
        {
            mapnik::new_geometry::linear_ring ring;
            int num_points = read_integer();
            if (num_points > 0)
            {
                ring.reserve(num_points);
                CoordinateArray ar(num_points);
                read_coords<M, Z>(ar);
                for (int j = 0; j < num_points ; ++j)
                {
                    ring.emplace_back(ar[j].x, ar[j].y);
                }
            }
            if ( i == 0) poly.set_exterior_ring(std::move(ring));
            else poly.add_hole(std::move(ring));
        }
        return poly;
    }

    template <bool M = false, bool Z = false>
    mapnik::new_geometry::multi_polygon read_multipolygon()
    {
        int num_polys = read_integer();
        mapnik::new_geometry::multi_polygon multi_poly;
        for (int i = 0; i < num_polys; ++i)
        {
            pos_ += 5;
            multi_poly.push_back(std::move(read_polygon<M, Z>()));
        }
        return multi_poly;
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

mapnik::new_geometry::geometry geometry_utils::from_wkb(const char* wkb,
                                                        unsigned size,
                                                        wkbFormat format)
{
    wkb_reader reader(wkb, size, format);
    return mapnik::new_geometry::geometry(reader.read());
}

} // namespace mapnik
