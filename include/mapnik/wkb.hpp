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

#ifndef MAPNIK_WKB_HPP
#define MAPNIK_WKB_HPP

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_container.hpp>
#include <mapnik/noncopyable.hpp>

namespace mapnik
{

/*!
 * From wikipedia.com:
 *
 * Well-known text (WKT) is a text markup language for representing vector
 * geometry objects on a map, spatial reference systems of spatial objects
 * and transformations between spatial reference systems. A binary equivalent,
 * known as well-known binary (WKB) is used to transfer and store the same
 * information on databases, such as PostGIS. The formats are regulated by
 * the Open Geospatial Consortium (OGC) and described in their Simple Feature
 * Access and Coordinate Transformation Service specifications.
 */
enum wkbFormat
{
    wkbAuto=1,
    wkbGeneric=2,
    wkbSpatiaLite=3
};

class MAPNIK_DECL geometry_utils : private mapnik::noncopyable
{
public:

    static bool from_wkb(mapnik::geometry_container& paths,
                          const char* wkb,
                          unsigned size,
                          wkbFormat format = wkbGeneric);
};
}

#endif // MAPNIK_WKB_HPP
