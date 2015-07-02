/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/geometry.hpp>

// ogr
#include "ogr_converter.hpp"
#include <ogr_core.h>
#include <ogr_geometry.h>

mapnik::geometry::geometry<double> ogr_converter::convert_geometry(OGRGeometry* ogr_geom)
{
    // NOTE: wkbFlatten macro in ogr flattens 2.5d types into base 2d type
    switch (wkbFlatten(ogr_geom->getGeometryType()))
    {
    case wkbPoint:
        return convert_point(static_cast<OGRPoint*>(ogr_geom));
        break;
    case wkbMultiPoint:
        return convert_multipoint(static_cast<OGRMultiPoint*>(ogr_geom));
        break;
    case wkbLinearRing:
        return convert_linestring(static_cast<OGRLinearRing*>(ogr_geom));
        break;
    case wkbLineString:
        return convert_linestring(static_cast<OGRLineString*>(ogr_geom));
        break;
    case wkbMultiLineString:
        return convert_multilinestring(static_cast<OGRMultiLineString*>(ogr_geom));
        break;
    case wkbPolygon:
        return convert_polygon(static_cast<OGRPolygon*>(ogr_geom));
        break;
    case wkbMultiPolygon:
        return convert_multipolygon(static_cast<OGRMultiPolygon*>(ogr_geom));
        break;
    case wkbGeometryCollection:
        return convert_collection(static_cast<OGRGeometryCollection*>(ogr_geom));
        break;
    case wkbNone:
    case wkbUnknown:
    default:
        {
            MAPNIK_LOG_WARN(ogr) << "ogr_converter: unknown <ogr> geometry_type="
                                 << wkbFlatten(ogr_geom->getGeometryType());
        }
        return mapnik::geometry::geometry<double>();
        break;
    }
}

mapnik::geometry::point<double> ogr_converter::convert_point(OGRPoint* ogr_geom)
{
    return mapnik::geometry::point<double>(ogr_geom->getX(), ogr_geom->getY());
}

mapnik::geometry::multi_point<double> ogr_converter::convert_multipoint(OGRMultiPoint* ogr_geom)
{
    mapnik::geometry::multi_point<double> geom;
    int num_geometries = ogr_geom->getNumGeometries();
    geom.reserve(num_geometries);
    for (int i = 0; i < num_geometries; ++i)
    {
        OGRPoint const* pt = static_cast<OGRPoint*>(ogr_geom->getGeometryRef(i));
        geom.emplace_back(pt->getX(), pt->getY());
    }
    return geom;
}

mapnik::geometry::line_string<double> ogr_converter::convert_linestring(OGRLineString* ogr_geom)
{
    mapnik::geometry::line_string<double> geom;
    int num_points = ogr_geom->getNumPoints();
    geom.reserve(num_points);
    for (int i = 0; i < num_points; ++i)
    {
       geom.add_coord(ogr_geom->getX(i), ogr_geom->getY(i));
    }
    return geom;
}

mapnik::geometry::multi_line_string<double> ogr_converter::convert_multilinestring(OGRMultiLineString* ogr_geom)
{
    mapnik::geometry::multi_line_string<double> geom;
    int num_geometries = ogr_geom->getNumGeometries();
    geom.reserve(num_geometries);
    for (int i = 0; i < num_geometries; ++i)
    {
        geom.emplace_back(convert_linestring(static_cast<OGRLineString*>(ogr_geom->getGeometryRef(i))));
    }
    return geom;
}


mapnik::geometry::polygon<double> ogr_converter::convert_polygon(OGRPolygon* ogr_geom)
{
    mapnik::geometry::polygon<double> geom;
    mapnik::geometry::linear_ring<double> exterior;
    OGRLinearRing* ogr_exterior = ogr_geom->getExteriorRing();
    int num_points = ogr_exterior->getNumPoints();
    exterior.reserve(num_points);
    for (int i = 0; i < num_points; ++i)
    {
        exterior.emplace_back(ogr_exterior->getX(i), ogr_exterior->getY(i));
    }
    geom.set_exterior_ring(std::move(exterior));

    int num_interior = ogr_geom->getNumInteriorRings();
    for (int r = 0; r < num_interior; ++r)
    {
        OGRLinearRing* ogr_interior = ogr_geom->getInteriorRing(r);
        mapnik::geometry::linear_ring<double> interior;
        int num_interior_points = ogr_interior->getNumPoints();
        interior.reserve(num_interior_points);
        for (int i = 0; i < num_interior_points; ++i)
        {
            interior.emplace_back(ogr_interior->getX(i), ogr_interior->getY(i));
        }
        geom.add_hole(std::move(interior));
    }
    return geom;
}

mapnik::geometry::multi_polygon<double> ogr_converter::convert_multipolygon(OGRMultiPolygon* ogr_geom)
{
    mapnik::geometry::multi_polygon<double> geom;
    int num_geometries = ogr_geom->getNumGeometries();
    geom.reserve(num_geometries);
    for (int i = 0; i < num_geometries; ++i)
    {
        geom.emplace_back(convert_polygon(static_cast<OGRPolygon*>(ogr_geom->getGeometryRef(i))));
    }
    return geom;
}

mapnik::geometry::geometry_collection<double> ogr_converter::convert_collection(OGRGeometryCollection* ogr_geom)
{
    mapnik::geometry::geometry_collection<double> geom;
    int num_geometries = ogr_geom->getNumGeometries();
    geom.reserve(num_geometries);
    for (int i = 0; i < num_geometries; ++i)
    {
        OGRGeometry* g = ogr_geom->getGeometryRef(i);
        if (g != nullptr)
        {
            geom.emplace_back(convert_geometry(g));
        }
    }
    return geom;
}
