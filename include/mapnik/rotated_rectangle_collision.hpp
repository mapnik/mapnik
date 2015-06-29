/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_ROTATED_RECTANGLE_COLLISION_HPP
#define MAPNIK_ROTATED_RECTANGLE_COLLISION_HPP

#include <mapnik/box2d.hpp>
#include <mapnik/geometry.hpp>
#include <array>
#include <cmath>
#include <iostream>

namespace mapnik {

template <typename T>
using rotated_rectangle = std::array<geometry::point<T> ,4>;

template <typename T>
rotated_rectangle<T> rotate(mapnik::box2d<T> const& box, double angle)
{
    rotated_rectangle<T> rect;
    auto c = box.center();
    auto lox = box.minx() - c.x;
    auto loy = box.miny() - c.y;
    auto hix = box.maxx() - c.x;
    auto hiy = box.maxy() - c.y;
    double cos_a = cos(angle);
    double sin_a = sin(angle);

    rect[0].x = c.x + lox * cos_a - loy * sin_a;
    rect[0].y = c.y + lox * sin_a + loy * cos_a;
    rect[1].x = c.x + lox * cos_a - hiy * sin_a;
    rect[1].y = c.y + lox * sin_a + hiy * cos_a;
    rect[2].x = c.x + hix * cos_a - hiy * sin_a;
    rect[2].y = c.y + hix * sin_a + hiy * cos_a;
    rect[3].x = c.x + hix * cos_a - loy * sin_a;
    rect[3].y = c.y + hix * sin_a + loy * cos_a;

    return rect;
}

template <typename T>
box2d<T> bounding_box(rotated_rectangle<T> const& r)
{
    box2d<T> box(r[0].x, r[0].x, r[0].y, r[0].y);
    box.expand_to_include(r[1].x, r[1].y);
    box.expand_to_include(r[2].x, r[2].y);
    box.expand_to_include(r[3].x, r[3].y);
    return box;
}

template <typename T>
bool intersects(rotated_rectangle<T> const& r0, rotated_rectangle<T> const& r1)
{
    for (rotated_rectangle<T> const& r : { std::cref(r0), std::cref(r1) })
    {
        for (std::size_t i = 0; i < 2; ++i)
        {
            auto nx = r[i + 1].y - r[i].y;
            auto ny = r[i].x - r[i + 1].x;
            auto min_a = std::numeric_limits<T>::max();
            auto max_a = -min_a;
            for(std::size_t j = 0; j < 4; ++j)
            {
                auto projected = nx * r0[j].x + ny * r0[j].y;
                if( projected < min_a ) min_a = projected;
                if( projected > max_a ) max_a = projected;
            }

            auto min_b = std::numeric_limits<T>::max();
            auto max_b = -min_b;
            for(std::size_t j = 0; j < 4; ++j)
            {
                auto projected = nx * r1[j].x + ny * r1[j].y;
                if( projected < min_b ) min_b = projected;
                if( projected > max_b ) max_b = projected;
            }
            if ( max_a < min_b || max_b < min_a ) return false;
        }
    }
    return true;
}

}

#endif //MAPNIK_ROTATED_RECTANGLE_COLLISION_HPP
