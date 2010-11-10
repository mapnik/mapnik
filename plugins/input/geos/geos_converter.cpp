/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2007 Artem Pavlenko
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
//$Id$

#include <mapnik/global.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_layer_desc.hpp>
#include <mapnik/wkb.hpp>
#include <mapnik/unicode.hpp>

// geos
#include "geos_converter.hpp"

using std::clog;
using std::endl;

using mapnik::feature_ptr;
using mapnik::geometry_utils;
using mapnik::geometry_type;


void geos_converter::convert_geometry (const GEOSGeometry* geom, feature_ptr feature, bool multiple_geometries)
{
    int type = GEOSGeomTypeId(geom);

    switch ( type )
    {
    case GEOS_POINT:
        convert_point (geom, feature);
        break;
    case GEOS_LINESTRING:
    case GEOS_LINEARRING:
        convert_linestring (geom, feature);
        break;
    case GEOS_POLYGON:
        convert_polygon (geom, feature);
        break;
    case GEOS_MULTIPOINT:
        if (multiple_geometries)
            convert_multipoint_2 (geom, feature);
        else
            // Todo - using convert_multipoint_2 until we have proper multipoint handling in convert_multipoint
            // http://trac.mapnik.org/ticket/458
            //convert_multipoint (geom, feature);
            convert_multipoint_2 (geom, feature);
        break;
    case GEOS_MULTILINESTRING:
        if (multiple_geometries)
            convert_multilinestring_2 (geom, feature);
        else
            convert_multilinestring (geom, feature);
        break;
    case GEOS_MULTIPOLYGON:
        if (multiple_geometries)
            convert_multipolygon_2 (geom, feature);
        else
            convert_multipolygon (geom, feature);
        break;
    case GEOS_GEOMETRYCOLLECTION:
        convert_collection (geom, feature, multiple_geometries);
        break;
    default:
#ifdef MAPNIK_DEBUG
        clog << "unknown <geos> geometry_type=" << type << endl;
#endif
        break;
    }
}

void geos_converter::convert_point (const GEOSGeometry* geom, feature_ptr feature)
{
    double x, y;
    unsigned int size;

    const GEOSCoordSequence* cs = GEOSGeom_getCoordSeq(geom);

    GEOSCoordSeq_getSize(cs, &size);
    GEOSCoordSeq_getX(cs, 0, &x);
    GEOSCoordSeq_getY(cs, 0, &y);

    geometry_type* point = new geometry_type(mapnik::Point);
    point->move_to (x, y);
    feature->add_geometry (point);
}

void geos_converter::convert_linestring (const GEOSGeometry* geom, feature_ptr feature)
{
    double x, y;
    unsigned int num_points;

    const GEOSCoordSequence* cs = GEOSGeom_getCoordSeq(geom);

    GEOSCoordSeq_getSize(cs, &num_points);
    GEOSCoordSeq_getX(cs, 0, &x);
    GEOSCoordSeq_getY(cs, 0, &y);

    geometry_type* line = new geometry_type(mapnik::LineString);
    line->set_capacity (num_points);
    line->move_to (x, y);

    for (unsigned int i = 1; i < num_points; ++i)
    {
        GEOSCoordSeq_getX(cs, i, &x);
        GEOSCoordSeq_getY(cs, i, &y);
        line->line_to (x, y);
    }
    feature->add_geometry (line);
}

void geos_converter::convert_polygon (const GEOSGeometry* geom, feature_ptr feature)
{
    double x, y;
    unsigned int num_points, num_interior;

    const GEOSGeometry* exterior = GEOSGetExteriorRing(geom);
    const GEOSCoordSequence* es = GEOSGeom_getCoordSeq(exterior);
    GEOSCoordSeq_getSize(es, &num_points);
    GEOSCoordSeq_getX(es, 0, &x);
    GEOSCoordSeq_getY(es, 0, &y);

    num_interior = GEOSGetNumInteriorRings(geom);

    unsigned int capacity = 0;
    for (unsigned int r = 0; r < num_interior; r++)
    {
        const GEOSGeometry* gtmp = GEOSGetInteriorRingN(geom, r);
        const GEOSCoordSequence* is = GEOSGeom_getCoordSeq(gtmp);
        
        unsigned int interior_size;
        GEOSCoordSeq_getSize(is, &interior_size);
        
        capacity += interior_size;
    }    

    geometry_type* poly = new geometry_type(mapnik::Polygon);
    poly->set_capacity (num_points + capacity);
    poly->move_to (x, y);
    for (unsigned int i = 1; i < num_points; ++i)
    {
        GEOSCoordSeq_getX(es, i, &x);
        GEOSCoordSeq_getY(es, i, &y);
        
        poly->line_to (x, y);
    }
    for (unsigned int r = 0; r < num_interior; ++r)
    {
        const GEOSGeometry* gtmp = GEOSGetInteriorRingN(geom, r);
        const GEOSCoordSequence* is = GEOSGeom_getCoordSeq(gtmp);
        
        GEOSCoordSeq_getSize(is, &num_points);
        GEOSCoordSeq_getX(is, 0, &x);
        GEOSCoordSeq_getY(is, 0, &y);

        poly->move_to(x, y);
        for (unsigned int i = 1; i < num_points; ++i)
        {
            GEOSCoordSeq_getX(is, i, &x);
            GEOSCoordSeq_getY(is, i, &y);

            poly->line_to(x, y);
        }
    }
    feature->add_geometry (poly);
}

void geos_converter::convert_multipoint (const GEOSGeometry* geom, feature_ptr feature)
{
    unsigned int num_geometries = GEOSGetNumGeometries(geom);
    geometry_type* point = new geometry_type(mapnik::Point);

    for (unsigned int i = 0; i < num_geometries; i++)
    {
        const GEOSGeometry* g = GEOSGetGeometryN(geom, i);

        if (g != NULL && GEOSisValid(g))
        {
            const GEOSCoordSequence* cs = GEOSGeom_getCoordSeq(geom);

            double x, y;
            //GEOSCoordSeq_getSize(cs, &size);
            GEOSCoordSeq_getX(cs, 0, &x);
            GEOSCoordSeq_getY(cs, 0, &y);

            point->move_to (x, y);
        }
    }

    feature->add_geometry (point);
}

void geos_converter::convert_multipoint_2 (const GEOSGeometry* geom, feature_ptr feature)
{
    unsigned int num_geometries = GEOSGetNumGeometries(geom);

    for (unsigned int i = 0; i < num_geometries; ++i)
    {
        const GEOSGeometry* g = GEOSGetGeometryN(geom, i);

        if (g != NULL && GEOSisValid(g))
        {
            convert_point (g, feature);
        }
    }
}

void geos_converter::convert_multilinestring (const GEOSGeometry* geom, feature_ptr feature)
{
/*
    int num_geometries = geom->getNumGeometries ();

    int num_points = 0;
    for (int i=0;i<num_geometries;i++)
    {
        OGRLineString* ls = static_cast<OGRLineString*>(geom->getGeometryRef (i));
        num_points += ls->getNumPoints ();
    }

    geometry_type* line = new geometry_type(mapnik::LineString);
    line->set_capacity (num_points);

    for (int i=0;i<num_geometries;i++)
    {
        OGRLineString* ls = static_cast<OGRLineString*>(geom->getGeometryRef (i));
        num_points = ls->getNumPoints ();
        line->move_to (ls->getX (0), ls->getY (0));
        for (int i=1;i<num_points;++i)
        {
            line->line_to (ls->getX (i), ls->getY (i));
        }
    }

    feature->add_geometry (line);
*/
}

void geos_converter::convert_multilinestring_2 (const GEOSGeometry* geom, feature_ptr feature)
{
    unsigned int num_geometries = GEOSGetNumGeometries(geom);

    for (unsigned int i = 0; i < num_geometries; ++i)
    {
        const GEOSGeometry* g = GEOSGetGeometryN(geom, i);

        if (g != NULL && GEOSisValid(g))
        {
            convert_linestring (g, feature);
        }
    }
}

void geos_converter::convert_multipolygon (const GEOSGeometry* geom, feature_ptr feature)
{
/*
    int num_geometries = geom->getNumGeometries ();

    int capacity = 0;
    for (int i=0;i<num_geometries;i++)
    {
        OGRPolygon* p = static_cast<OGRPolygon*>(geom->getGeometryRef (i));
        OGRLinearRing* exterior = p->getExteriorRing ();
        capacity += exterior->getNumPoints ();
        for (int r=0;r<p->getNumInteriorRings ();r++)
        {
            OGRLinearRing* interior = p->getInteriorRing (r);
            capacity += interior->getNumPoints ();
        }    
    }

    geometry_type* poly = new geometry_type(mapnik::Polygon);
    poly->set_capacity (capacity);

    for (int i=0;i<num_geometries;i++)
    {
        OGRPolygon* p = static_cast<OGRPolygon*>(geom->getGeometryRef (i));
        OGRLinearRing* exterior = p->getExteriorRing ();
        int num_points = exterior->getNumPoints ();
        int num_interior = p->getNumInteriorRings ();
        poly->move_to (exterior->getX (0), exterior->getY (0));
        for (int i=1;i<num_points;++i)
        {
            poly->line_to (exterior->getX (i), exterior->getY (i));
        }
        for (int r=0;r<num_interior;r++)
        {
            OGRLinearRing* interior = p->getInteriorRing (r);
            num_points = interior->getNumPoints ();
            poly->move_to(interior->getX (0), interior->getY (0));
            for (int i=1;i<num_points;++i)
            {
                poly->line_to(interior->getX (i), interior->getY (i));
            }
        }
    }

    feature->add_geometry (poly);
*/
}

void geos_converter::convert_multipolygon_2 (const GEOSGeometry* geom, feature_ptr feature)
{
    unsigned int num_geometries = GEOSGetNumGeometries(geom);

    for (unsigned int i = 0; i < num_geometries; ++i)
    {
        const GEOSGeometry* g = GEOSGetGeometryN(geom, i);

        if (g != NULL && GEOSisValid(g))
        {
            convert_polygon (g, feature);
        }
    }
}

void geos_converter::convert_collection (const GEOSGeometry* geom, feature_ptr feature, bool multiple_geometries)
{
    unsigned int num_geometries = GEOSGetNumGeometries(geom);

    for (unsigned int i = 0; i < num_geometries; ++i)
    {
        const GEOSGeometry* g = GEOSGetGeometryN(geom, i);

        if (g != NULL && GEOSisValid(g))
        {
            convert_geometry (g, feature, multiple_geometries);
        }
    }
}

