/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/geometry.hpp>

class OGRGeometry;
class OGRGeometryCollection;
class OGRLineString;
class OGRMultiLineString;
class OGRMultiPoint;
class OGRMultiPolygon;
class OGRPoint;
class OGRPolygon;

class ogr_converter
{
  public:

    static mapnik::geometry::geometry<double> convert_geometry(OGRGeometry* ogr_geom);
    static mapnik::geometry::point<double> convert_point(OGRPoint* ogr_geom);
    static mapnik::geometry::multi_point<double> convert_multipoint(OGRMultiPoint* ogr_geom);
    static mapnik::geometry::line_string<double> convert_linestring(OGRLineString* ogr_geom);
    static mapnik::geometry::multi_line_string<double> convert_multilinestring(OGRMultiLineString* ogr_geom);
    static mapnik::geometry::polygon<double> convert_polygon(OGRPolygon* ogr_geom);
    static mapnik::geometry::multi_polygon<double> convert_multipolygon(OGRMultiPolygon* ogr_geom);
    static mapnik::geometry::geometry_collection<double> convert_collection(OGRGeometryCollection* ogr_geom);
};

#endif // OGR_CONVERTER_HPP
