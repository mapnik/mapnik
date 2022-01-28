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

#ifndef MAPNIK_GEOMETRY_TO_WKT_HPP
#define MAPNIK_GEOMETRY_TO_WKT_HPP

// mapnik
#include <mapnik/geometry.hpp>
// stl
#include <string>

namespace mapnik {
namespace util {

bool to_wkt(std::string& wkt, mapnik::geometry::geometry<double> const& geom);

bool to_wkt(std::string& wkt, mapnik::geometry::geometry<std::int64_t> const& geom);

} // namespace util
} // namespace mapnik

#endif // MAPNIK_GEOMETRY_TO_WKT_HPP
