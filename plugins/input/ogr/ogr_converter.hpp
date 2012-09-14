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

#ifndef OGR_CONVERTER_HPP
#define OGR_CONVERTER_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/params.hpp>

// ogr
#include <ogrsf_frmts.h>

class ogr_converter
{
public:

    static void convert_geometry (OGRGeometry* geom, mapnik::feature_ptr feature);
    static void convert_collection (OGRGeometryCollection* geom, mapnik::feature_ptr feature);
    static void convert_point (OGRPoint* geom, mapnik::feature_ptr feature);
    static void convert_linestring (OGRLineString* geom, mapnik::feature_ptr feature);
    static void convert_polygon (OGRPolygon* geom, mapnik::feature_ptr feature);
    static void convert_multipoint (OGRMultiPoint* geom, mapnik::feature_ptr feature);
    static void convert_multilinestring (OGRMultiLineString* geom, mapnik::feature_ptr feature);
    static void convert_multipolygon (OGRMultiPolygon* geom, mapnik::feature_ptr feature);
};

#endif // OGR_CONVERTER_HPP
