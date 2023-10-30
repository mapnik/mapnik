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

#ifndef MAPNIK_GEOMETRY_CLOSEST_POINT_HPP
#define MAPNIK_GEOMETRY_CLOSEST_POINT_HPP

#include <boost/version.hpp>

#if BOOST_VERSION >= 106200

#include <mapnik/geometry.hpp>
#include <mapnik/geometry/boost_adapters.hpp>

namespace mapnik {
namespace geometry {
using coordinate_type = double;

struct closest_point_result
{
    double x = 0.0;
    double y = 0.0;
    double distance = -1.0;
};

using result_type = closest_point_result;
template<typename T1, typename T2>
MAPNIK_DECL result_type closest_point(T1 const& geom, mapnik::geometry::point<T2> const& pt);

} // namespace geometry
} // namespace mapnik

#endif //
#endif // MAPNIK_GEOMETRY_CLOSEST_POINT_HPP
