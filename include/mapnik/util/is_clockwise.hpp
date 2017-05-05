/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_UTIL_IS_CLOCKWISE_HPP
#define MAPNIK_UTIL_IS_CLOCKWISE_HPP

#include <cassert>

namespace mapnik { namespace util {

template <typename T>
bool is_clockwise(T const& ring)
{
    double area = 0.0;
    std::size_t num_points = ring.size();
    assert(num_points > 2);
    double orig_x = ring[0].x;
    double orig_y = ring[0].y;
    for (std::size_t i = 0; i < num_points; ++i)
    {
        auto const& p0 = ring[i];
        auto const& p1 = ring[(i + 1) % num_points];
        double x0 = p0.x - orig_x;
        double y0 = p0.y - orig_y;
        double x1 = p1.x - orig_x;
        double y1 = p1.y - orig_y;
        area += x0 * y1 - x1 * y0;
    }
    return (area < 0.0) ? true : false;
}
}}

#endif //MAPNIK_UTIL_IS_CLOCKWISE_HPP
