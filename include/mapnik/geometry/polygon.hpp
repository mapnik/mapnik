/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_GEOMETRY_POLYGON_HPP
#define MAPNIK_GEOMETRY_POLYGON_HPP

// geometry
#include <mapbox/geometry/polygon.hpp>

namespace mapnik {
namespace geometry {

template<typename T>
using linear_ring = mapbox::geometry::linear_ring<T>;

template<typename T>
using polygon = mapbox::geometry::polygon<T>;

} // namespace geometry
} // namespace mapnik

#endif // MAPNIK_GEOMETRY_POLYGON_HPP
