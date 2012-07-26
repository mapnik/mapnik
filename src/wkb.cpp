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
#include <mapnik/debug.hpp>
#include <mapnik/global.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/coord_array.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/feature.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/format.hpp>

namespace mapnik
{

typedef coord_array<coord2d> CoordinateArray;

struct wkb_reader : boost::noncopyable
{
private:
    enum wkbByteOrder {
        wkbXDR=0,
        wkbNDR=1
    };

    const char* wkb_;
    unsigned size_;
    unsigned pos_;
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
        wkbPointZ=1001,
        wkbLineStringZ=1002,
        wkbPolygonZ=1003,
        wkbMultiPointZ=1004,
        wkbMultiLineStringZ=1005,
        wkbMultiPolygonZ=1006,
        wkbGeometryCollectionZ=1007
    };

    wkb_reader(const char* wkb, unsigned size, wkbFormat format)
        : wkb_(wkb),
          size_(size),
          pos_(0),
          format_(format)
    {
        // try to determine WKB format automatically
        if (format_ == wkbAuto)
        {
            if (size_ >= 44
                && (unsigned char)(wkb_[0]) == (unsigned char)(0x00)
                && (unsigned char)(wkb_[38]) == (unsigned char)(0x7C))
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

#ifndef MAPNIK_BIG_ENDIAN
        needSwap_ = byteOrder_ ? wkbXDR : wkbNDR;
#else
        needSwap_ = byteOrder_ ? wkbNDR : wkbXDR;
#endif
    }

    void read(boost::ptr_vector<geometry_type> & paths)
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
            read_point_xyz(paths);
            break;
        case wkbLineStringZ:
            read_linestring_xyz(paths);
            break;
        case wkbPolygonZ:
            read_polygon_xyz(paths);
            break;
        case wkbMultiPointZ:
            read_multipoint_xyz(paths);
            break;
        case wkbMultiLineStringZ:
            read_multilinestring_xyz(paths);
            break;
        case wkbMultiPolygonZ:
            read_multipolygon_xyz(paths);
            break;
        case wkbGeometryCollectionZ:
            read_collection(paths);
            break;
        default:
            break;
        }
    }

private:

    int read_integer()
    {
        boost::int32_t n;
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
            for (unsigned i = 0; i < ar.size(); ++i)
            {
                read_double_ndr(wkb_ + pos_, ar[i].x);
                read_double_ndr(wkb_ + pos_ + 8, ar[i].y);
                pos_ += 16; // skip XY
            }
        }
        else
        {
            for (unsigned i=0;i<ar.size();++i)
            {
                read_double_xdr(wkb_ + pos_, ar[i].x);
                read_double_xdr(wkb_ + pos_ + 8, ar[i].y);
                pos_ += 16; // skip XY
            }
        }
    }

    void read_coords_xyz(CoordinateArray& ar)
    {
        if (! needSwap_)
        {
            for (unsigned i = 0; i < ar.size(); ++i)
            {
                read_double_ndr(wkb_ + pos_, ar[i].x);
                read_double_ndr(wkb_ + pos_ + 8, ar[i].y);
                pos_ += 24; // skip XYZ
            }
        }
        else
        {
            for (unsigned i = 0; i < ar.size(); ++i)
            {
                read_double_xdr(wkb_ + pos_, ar[i].x);
                read_double_xdr(wkb_ + pos_ + 8, ar[i].y);
                pos_ += 24; // skip XYZ
            }
        }
    }


    void read_point(boost::ptr_vector<geometry_type> & paths)
    {
        double x = read_double();
        double y = read_double();
        std::auto_ptr<geometry_type> pt(new geometry_type(Point));
        pt->move_to(x, y);
        paths.push_back(pt);
    }

    void read_multipoint(boost::ptr_vector<geometry_type> & paths)
    {
        int num_points = read_integer();
        for (int i = 0; i < num_points; ++i)
        {
            pos_ += 5;
            read_point(paths);
        }
    }

    void read_point_xyz(boost::ptr_vector<geometry_type> & paths)
    {
        double x = read_double();
        double y = read_double();
        std::auto_ptr<geometry_type> pt(new geometry_type(Point));
        pos_ += 8; // double z = read_double();
        pt->move_to(x, y);
        paths.push_back(pt);
    }

    void read_multipoint_xyz(boost::ptr_vector<geometry_type> & paths)
    {
        int num_points = read_integer();
        for (int i = 0; i < num_points; ++i)
        {
            pos_ += 5;
            read_point_xyz(paths);
        }
    }

    void read_linestring(boost::ptr_vector<geometry_type> & paths)
    {
        int num_points = read_integer();
        if (num_points > 0)
        {
            CoordinateArray ar(num_points);
            read_coords(ar);
            std::auto_ptr<geometry_type> line(new geometry_type(LineString));
            line->move_to(ar[0].x, ar[0].y);
            for (int i = 1; i < num_points; ++i)
            {
                line->line_to(ar[i].x, ar[i].y);
            }
            paths.push_back(line);
        }
    }

    void read_multilinestring(boost::ptr_vector<geometry_type> & paths)
    {
        int num_lines = read_integer();
        for (int i = 0; i < num_lines; ++i)
        {
            pos_ += 5;
            read_linestring(paths);
        }
    }

    void read_linestring_xyz(boost::ptr_vector<geometry_type> & paths)
    {
        int num_points = read_integer();
        if (num_points > 0)
        {
            CoordinateArray ar(num_points);
            read_coords_xyz(ar);
            std::auto_ptr<geometry_type> line(new geometry_type(LineString));
            line->move_to(ar[0].x, ar[0].y);
            for (int i = 1; i < num_points; ++i)
            {
                line->line_to(ar[i].x, ar[i].y);
            }
            paths.push_back(line);
        }
    }

    void read_multilinestring_xyz(boost::ptr_vector<geometry_type> & paths)
    {
        int num_lines = read_integer();
        for (int i = 0; i < num_lines; ++i)
        {
            pos_ += 5;
            read_linestring_xyz(paths);
        }
    }


    void read_polygon(boost::ptr_vector<geometry_type> & paths)
    {
        int num_rings = read_integer();
        if (num_rings > 0)
        {
            std::auto_ptr<geometry_type> poly(new geometry_type(Polygon));
            for (int i = 0; i < num_rings; ++i)
            {
                int num_points = read_integer();
                if (num_points > 0)
                {
                    CoordinateArray ar(num_points);
                    read_coords(ar);
                    poly->move_to(ar[0].x, ar[0].y);
                    for (int j = 1; j < num_points - 1; ++j)
                    {
                        poly->line_to(ar[j].x, ar[j].y);
                    }
                    poly->close(ar[num_points-1].x, ar[num_points-1].y);
                }
            }
            if (poly->size() > 2) // ignore if polygon has less than 3 vertices
                paths.push_back(poly);
        }
    }

    void read_multipolygon(boost::ptr_vector<geometry_type> & paths)
    {
        int num_polys = read_integer();
        for (int i = 0; i < num_polys; ++i)
        {
            pos_ += 5;
            read_polygon(paths);
        }
    }

    void read_polygon_xyz(boost::ptr_vector<geometry_type> & paths)
    {
        int num_rings = read_integer();
        if (num_rings > 0)
        {
            std::auto_ptr<geometry_type> poly(new geometry_type(Polygon));
            for (int i = 0; i < num_rings; ++i)
            {
                int num_points = read_integer();
                if (num_points > 0)
                {
                    CoordinateArray ar(num_points);
                    read_coords_xyz(ar);
                    poly->move_to(ar[0].x, ar[0].y);
                    for (int j = 1; j < num_points - 1; ++j)
                    {
                        poly->line_to(ar[j].x, ar[j].y);
                    }
                    poly->close(ar[num_points-1].x, ar[num_points-1].y);
                }
            }
            if (poly->size() > 2) // ignore if polygon has less than 3 vertices
                paths.push_back(poly);
        }
    }

    void read_multipolygon_xyz(boost::ptr_vector<geometry_type> & paths)
    {
        int num_polys = read_integer();
        for (int i = 0; i < num_polys; ++i)
        {
            pos_ += 5;
            read_polygon_xyz(paths);
        }
    }

    void read_collection(boost::ptr_vector<geometry_type> & paths)
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
        case wkbLineString:          s << "LineString"; break;
        case wkbPolygon:             s << "Polygon"; break;
        case wkbMultiPoint:          s << "MultiPoint"; break;
        case wkbMultiLineString:     s << "MultiLineString"; break;
        case wkbMultiPolygon:        s << "MultiPolygon"; break;
        case wkbGeometryCollection:  s << "GeometryCollection"; break;
        case wkbPointZ:              s << "PointZ"; break;
        case wkbLineStringZ:         s << "LineStringZ"; break;
        case wkbPolygonZ:            s << "PolygonZ"; break;
        case wkbMultiPointZ:         s << "MultiPointZ"; break;
        case wkbMultiLineStringZ:    s << "MultiLineStringZ"; break;
        case wkbMultiPolygonZ:       s << "MultiPolygonZ"; break;
        case wkbGeometryCollectionZ: s << "GeometryCollectionZ"; break;
        default:                     s << "wkbUknown(" << type << ")"; break;
        }

        return s.str();
    }

};

bool geometry_utils::from_wkb(boost::ptr_vector<geometry_type>& paths,
                               const char* wkb,
                               unsigned size,
                               wkbFormat format)
{
    unsigned geom_count = paths.size();
    wkb_reader reader(wkb, size, format);
    reader.read(paths);
    if (paths.size() > geom_count)
        return true;
    return false;
}

}
