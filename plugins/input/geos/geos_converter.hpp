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

#ifndef GEOS_CONVERTER_HPP
#define GEOS_CONVERTER_HPP

// mapnik
#include <mapnik/datasource.hpp>

// geos
#include <geos_c.h>
  
class geos_converter
{
public:

    static void convert_geometry (const GEOSGeometry* geom, mapnik::feature_ptr feature, bool multiple_geometries);
    static void convert_collection (const GEOSGeometry* geom, mapnik::feature_ptr feature, bool multiple_geometries);
    static void convert_point (const GEOSGeometry* geom, mapnik::feature_ptr feature);
    static void convert_linestring (const GEOSGeometry* geom, mapnik::feature_ptr feature);
    static void convert_polygon (const GEOSGeometry* geom, mapnik::feature_ptr feature);
    static void convert_multipoint (const GEOSGeometry* geom, mapnik::feature_ptr feature);
    static void convert_multipoint_2 (const GEOSGeometry* geom, mapnik::feature_ptr feature);
    static void convert_multilinestring (const GEOSGeometry* geom, mapnik::feature_ptr feature);
    static void convert_multilinestring_2 (const GEOSGeometry* geom, mapnik::feature_ptr feature);
    static void convert_multipolygon (const GEOSGeometry* geom, mapnik::feature_ptr feature);
    static void convert_multipolygon_2 (const GEOSGeometry* geom, mapnik::feature_ptr feature);
};

#endif // GEOS_FEATURESET_HPP
