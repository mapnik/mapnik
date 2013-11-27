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
#include <mapnik/global.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/unicode.hpp>

// ogr
#include "ogr_converter.hpp"

using mapnik::feature_ptr;
using mapnik::geometry_utils;
using mapnik::geometry_type;

void ogr_converter::convert_geometry(OGRGeometry* geom, feature_ptr feature)
{
    // NOTE: wkbFlatten macro in ogr flattens 2.5d types into base 2d type
    switch (wkbFlatten(geom->getGeometryType()))
    {
    case wkbPoint:
        convert_point(static_cast<OGRPoint*>(geom), feature);
        break;
    case wkbMultiPoint:
        convert_multipoint(static_cast<OGRMultiPoint*>(geom), feature);
        break;
    case wkbLinearRing:
        convert_linestring(static_cast<OGRLinearRing*>(geom), feature);
        break;
    case wkbLineString:
        convert_linestring(static_cast<OGRLineString*>(geom), feature);
        break;
    case wkbMultiLineString:
        convert_multilinestring(static_cast<OGRMultiLineString*>(geom), feature);
        break;
    case wkbPolygon:
        convert_polygon(static_cast<OGRPolygon*>(geom), feature);
        break;
    case wkbMultiPolygon:
        convert_multipolygon(static_cast<OGRMultiPolygon*>(geom), feature);
        break;
    case wkbGeometryCollection:
        convert_collection(static_cast<OGRGeometryCollection*>(geom), feature);
        break;
    case wkbNone:
    case wkbUnknown:
    default:
        {
            MAPNIK_LOG_WARN(ogr) << "ogr_converter: unknown <ogr> geometry_type="
                                 << wkbFlatten(geom->getGeometryType());
        }
        break;
    }
}

void ogr_converter::convert_point(OGRPoint* geom, feature_ptr feature)
{
    std::unique_ptr<geometry_type> point(new geometry_type(mapnik::geometry_type::types::Point));
    point->move_to(geom->getX(), geom->getY());
    feature->paths().push_back(point.release());
}

void ogr_converter::convert_linestring(OGRLineString* geom, feature_ptr feature)
{
    int num_points = geom->getNumPoints();
    std::unique_ptr<geometry_type> line(new geometry_type(mapnik::geometry_type::types::LineString));
    line->move_to(geom->getX(0), geom->getY(0));
    for (int i = 1; i < num_points; ++i)
    {
        line->line_to (geom->getX(i), geom->getY(i));
    }
    feature->paths().push_back(line.release());
}

void ogr_converter::convert_polygon(OGRPolygon* geom, feature_ptr feature)
{
    OGRLinearRing* exterior = geom->getExteriorRing();
    int num_points = exterior->getNumPoints();
    int num_interior = geom->getNumInteriorRings();
    int capacity = 0;
    for (int r = 0; r < num_interior; ++r)
    {
        OGRLinearRing* interior = geom->getInteriorRing(r);
        capacity += interior->getNumPoints();
    }

    std::unique_ptr<geometry_type> poly(new geometry_type(mapnik::geometry_type::types::Polygon));

    poly->move_to(exterior->getX(0), exterior->getY(0));
    for (int i = 1; i < num_points; ++i)
    {
        poly->line_to(exterior->getX(i), exterior->getY(i));
    }
    poly->close_path();
    for (int r = 0; r < num_interior; ++r)
    {
        OGRLinearRing* interior = geom->getInteriorRing(r);
        num_points = interior->getNumPoints();
        poly->move_to(interior->getX(0), interior->getY(0));
        for (int i = 1; i < num_points; ++i)
        {
            poly->line_to(interior->getX(i), interior->getY(i));
        }
        poly->close_path();
    }
    feature->paths().push_back(poly.release());
}

void ogr_converter::convert_multipoint(OGRMultiPoint* geom, feature_ptr feature)
{
    int num_geometries = geom->getNumGeometries();
    for (int i = 0; i < num_geometries; ++i)
    {
        convert_point(static_cast<OGRPoint*>(geom->getGeometryRef(i)), feature);
    }
}

void ogr_converter::convert_multilinestring(OGRMultiLineString* geom, feature_ptr feature)
{
    int num_geometries = geom->getNumGeometries();
    for (int i = 0; i < num_geometries; ++i)
    {
        convert_linestring(static_cast<OGRLineString*>(geom->getGeometryRef(i)), feature);
    }
}

void ogr_converter::convert_multipolygon(OGRMultiPolygon* geom, feature_ptr feature)
{
    int num_geometries = geom->getNumGeometries();
    for (int i = 0; i < num_geometries; ++i)
    {
        convert_polygon(static_cast<OGRPolygon*>(geom->getGeometryRef(i)), feature);
    }
}

void ogr_converter::convert_collection(OGRGeometryCollection* geom, feature_ptr feature)
{
    int num_geometries = geom->getNumGeometries();
    for (int i = 0; i < num_geometries; ++i)
    {
        OGRGeometry* g = geom->getGeometryRef(i);
        if (g != nullptr)
        {
            convert_geometry(g, feature);
        }
    }
}
