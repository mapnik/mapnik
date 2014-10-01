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
#include <mapnik/make_unique.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/global.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/coord_array.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/noncopyable.hpp>

namespace mapnik
{

using CoordinateArray = coord_array<coord2d>;

struct wkb_reader : mapnik::noncopyable
{
private:
    enum wkbByteOrder {
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

    void read(geometry_container & paths)
    {
        int type = read_integer();

        switch (type)
        {
        case wkbPoint:
            read_point(paths);
            break;
        case wkbLineString:
            read_linestring(paths);
            break;
        case wkbPolygon:
            read_polygon(paths);
            break;
        case wkbMultiPoint:
            read_multipoint(paths);
            break;
        case wkbMultiLineString:
            read_multilinestring(paths);
            break;
        case wkbMultiPolygon:
            read_multipolygon(paths);
            break;
        case wkbGeometryCollection:
            read_collection(paths);
            break;
        case wkbPointZ:
        case wkbPointM:
            read_point_xyz(paths);
            break;
        case wkbPointZM:
            read_point_xyzm(paths);
            break;
        case wkbLineStringZ:
        case wkbLineStringM:
            read_linestring_xyz(paths);
            break;
        case wkbLineStringZM:
            read_linestring_xyzm(paths);
            break;
        case wkbPolygonZ:
        case wkbPolygonM:
            read_polygon_xyz(paths);
            break;
        case wkbPolygonZM:
            read_polygon_xyzm(paths);
            break;
        case wkbMultiPointZ:
        case wkbMultiPointM:
            read_multipoint_xyz(paths);
            break;
        case wkbMultiPointZM:
            read_multipoint_xyzm(paths);
            break;
        case wkbMultiLineStringZ:
        case wkbMultiLineStringM:
            read_multilinestring_xyz(paths);
            break;
        case wkbMultiLineStringZM:
            read_multilinestring_xyzm(paths);
            break;
        case wkbMultiPolygonZ:
        case wkbMultiPolygonM:
            read_multipolygon_xyz(paths);
            break;
        case wkbMultiPolygonZM:
            read_multipolygon_xyzm(paths);
            break;
        case wkbGeometryCollectionZ:
        case wkbGeometryCollectionM:
        case wkbGeometryCollectionZM:
            read_collection(paths);
            break;
        default:
            break;
        }
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

    void read_coords(CoordinateArray& ar)
    {
        if (! needSwap_)
        {
            for (auto & coord : ar)
            {
                read_double_ndr(wkb_ + pos_, coord.x);
                read_double_ndr(wkb_ + pos_ + 8, coord.y);
                pos_ += 16; // skip XY
            }
        }
        else
        {
            for (auto & coord : ar)
            {
                read_double_xdr(wkb_ + pos_, coord.x);
                read_double_xdr(wkb_ + pos_ + 8, coord.y);
                pos_ += 16; // skip XY
            }
        }
    }

    void read_coords_xyz(CoordinateArray& ar)
    {
        if (! needSwap_)
        {
            for (auto & coord : ar)
            {
                read_double_ndr(wkb_ + pos_, coord.x);
                read_double_ndr(wkb_ + pos_ + 8, coord.y);
                pos_ += 24; // skip XYZ
            }
        }
        else
        {
            for (auto & coord : ar)
            {
                read_double_xdr(wkb_ + pos_, coord.x);
                read_double_xdr(wkb_ + pos_ + 8, coord.y);
                pos_ += 24; // skip XYZ
            }
        }
    }

    void read_coords_xyzm(CoordinateArray& ar)
    {
        if (! needSwap_)
        {
            for (auto & coord : ar)
            {
                read_double_ndr(wkb_ + pos_, coord.x);
                read_double_ndr(wkb_ + pos_ + 8, coord.y);
                pos_ += 32; // skip XYZM
            }
        }
        else
        {
            for (auto & coord : ar)
            {
                read_double_xdr(wkb_ + pos_, coord.x);
                read_double_xdr(wkb_ + pos_ + 8, coord.y);
                pos_ += 32; // skip XYZM
            }
        }
    }

    void read_point(geometry_container & paths)
    {
        double x = read_double();
        double y = read_double();
        auto pt = std::make_unique<geometry_type>(geometry_type::types::Point);
        pt->move_to(x, y);
        paths.push_back(pt.release());
    }

    void read_multipoint(geometry_container & paths)
    {
        int num_points = read_integer();
        for (int i = 0; i < num_points; ++i)
        {
            pos_ += 5;
            read_point(paths);
        }
    }

    void read_point_xyz(geometry_container & paths)
    {
        double x = read_double();
        double y = read_double();
        auto pt = std::make_unique<geometry_type>(geometry_type::types::Point);
        pos_ += 8; // double z = read_double();
        pt->move_to(x, y);
        paths.push_back(pt.release());
    }

    void read_point_xyzm(geometry_container & paths)
    {
        double x = read_double();
        double y = read_double();
        auto pt = std::make_unique<geometry_type>(geometry_type::types::Point);
        pos_ += 16;
        pt->move_to(x, y);
        paths.push_back(pt.release());
    }

    void read_multipoint_xyz(geometry_container & paths)
    {
        int num_points = read_integer();
        for (int i = 0; i < num_points; ++i)
        {
            pos_ += 5;
            read_point_xyz(paths);
        }
    }

    void read_multipoint_xyzm(geometry_container & paths)
    {
        int num_points = read_integer();
        for (int i = 0; i < num_points; ++i)
        {
            pos_ += 5;
            read_point_xyzm(paths);
        }
    }

    void read_linestring(geometry_container & paths)
    {
        int num_points = read_integer();
        if (num_points > 0)
        {
            CoordinateArray ar(num_points);
            read_coords(ar);
            auto line = std::make_unique<geometry_type>(geometry_type::types::LineString);
            line->move_to(ar[0].x, ar[0].y);
            for (int i = 1; i < num_points; ++i)
            {
                line->line_to(ar[i].x, ar[i].y);
            }
            paths.push_back(line.release());
        }
    }

    void read_multilinestring(geometry_container & paths)
    {
        int num_lines = read_integer();
        for (int i = 0; i < num_lines; ++i)
        {
            pos_ += 5;
            read_linestring(paths);
        }
    }

    void read_linestring_xyz(geometry_container & paths)
    {
        int num_points = read_integer();
        if (num_points > 0)
        {
            CoordinateArray ar(num_points);
            read_coords_xyz(ar);
            auto line = std::make_unique<geometry_type>(geometry_type::types::LineString);
            line->move_to(ar[0].x, ar[0].y);
            for (int i = 1; i < num_points; ++i)
            {
                line->line_to(ar[i].x, ar[i].y);
            }
            paths.push_back(line.release());
        }
    }

    void read_linestring_xyzm(geometry_container & paths)
    {
        int num_points = read_integer();
        if (num_points > 0)
        {
            CoordinateArray ar(num_points);
            read_coords_xyzm(ar);
            auto line = std::make_unique<geometry_type>(geometry_type::types::LineString);
            line->move_to(ar[0].x, ar[0].y);
            for (int i = 1; i < num_points; ++i)
            {
                line->line_to(ar[i].x, ar[i].y);
            }
            paths.push_back(line.release());
        }
    }

    void read_multilinestring_xyz(geometry_container & paths)
    {
        int num_lines = read_integer();
        for (int i = 0; i < num_lines; ++i)
        {
            pos_ += 5;
            read_linestring_xyz(paths);
        }
    }

    void read_multilinestring_xyzm(geometry_container & paths)
    {
        int num_lines = read_integer();
        for (int i = 0; i < num_lines; ++i)
        {
            pos_ += 5;
            read_linestring_xyzm(paths);
        }
    }

    void read_polygon(geometry_container & paths)
    {
        int num_rings = read_integer();
        if (num_rings > 0)
        {
            auto poly = std::make_unique<geometry_type>(geometry_type::types::Polygon);
            for (int i = 0; i < num_rings; ++i)
            {
                int num_points = read_integer();
                if (num_points > 0)
                {
                    CoordinateArray ar(num_points);
                    read_coords(ar);
                    poly->move_to(ar[0].x, ar[0].y);
                    for (int j = 1; j < num_points ; ++j)
                    {
                        poly->line_to(ar[j].x, ar[j].y);
                    }
                    poly->close_path();
                }
            }
            if (poly->size() > 3) // ignore if polygon has less than (3 + close_path) vertices
                paths.push_back(poly.release());
        }
    }

    void read_multipolygon(geometry_container & paths)
    {
        int num_polys = read_integer();
        for (int i = 0; i < num_polys; ++i)
        {
            pos_ += 5;
            read_polygon(paths);
        }
    }

    void read_polygon_xyz(geometry_container & paths)
    {
        int num_rings = read_integer();
        if (num_rings > 0)
        {
            auto poly = std::make_unique<geometry_type>(geometry_type::types::Polygon);
            for (int i = 0; i < num_rings; ++i)
            {
                int num_points = read_integer();
                if (num_points > 0)
                {
                    CoordinateArray ar(num_points);
                    read_coords_xyz(ar);
                    poly->move_to(ar[0].x, ar[0].y);
                    for (int j = 1; j < num_points; ++j)
                    {
                        poly->line_to(ar[j].x, ar[j].y);
                    }
                    poly->close_path();
                }
            }
            if (poly->size() > 2) // ignore if polygon has less than 3 vertices
                paths.push_back(poly.release());
        }
    }

    void read_polygon_xyzm(geometry_container & paths)
    {
        int num_rings = read_integer();
        if (num_rings > 0)
        {
            auto poly = std::make_unique<geometry_type>(geometry_type::types::Polygon);
            for (int i = 0; i < num_rings; ++i)
            {
                int num_points = read_integer();
                if (num_points > 0)
                {
                    CoordinateArray ar(num_points);
                    read_coords_xyzm(ar);
                    poly->move_to(ar[0].x, ar[0].y);
                    for (int j = 1; j < num_points; ++j)
                    {
                        poly->line_to(ar[j].x, ar[j].y);
                    }
                    poly->close_path();
                }
            }
            if (poly->size() > 2) // ignore if polygon has less than 3 vertices
                paths.push_back(poly.release());
        }
    }

    void read_multipolygon_xyz(geometry_container & paths)
    {
        int num_polys = read_integer();
        for (int i = 0; i < num_polys; ++i)
        {
            pos_ += 5;
            read_polygon_xyz(paths);
        }
    }

    void read_multipolygon_xyzm(geometry_container & paths)
    {
        int num_polys = read_integer();
        for (int i = 0; i < num_polys; ++i)
        {
            pos_ += 5;
            read_polygon_xyzm(paths);
        }
    }

    void read_collection(geometry_container & paths)
    {
        int num_geometries = read_integer();
        for (int i = 0; i < num_geometries; ++i)
        {
            pos_ += 1; // skip byte order
            read(paths);
        }
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
        case wkbGeometryCollectionZM: s << "GeometryCollectionZM"; break;
        default:                     s << "wkbUnknown(" << type << ")"; break;
        }

        return s.str();
    }

};

bool geometry_utils::from_wkb(geometry_container& paths,
                               const char* wkb,
                               unsigned size,
                               wkbFormat format)
{
    std::size_t geom_count = paths.size();
    wkb_reader reader(wkb, size, format);
    reader.read(paths);
    if (paths.size() > geom_count)
        return true;
    return false;
}

}
