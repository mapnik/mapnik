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

#ifndef MAPNIK_MATH_HPP
#define MAPNIK_MATH_HPP

#include <mapnik/config.hpp>

namespace mapnik { namespace util {

constexpr double pi = 3.1415926535897932384626433832795;
constexpr double tau = 6.283185307179586476925286766559;

template <typename T>
constexpr T const& clamp(T const& v, T const& lo, T const& hi)
{
    return v < lo ? lo : hi < v ? hi : v;
}

constexpr double degrees(double rad)
{
    return rad * (360 / tau);
}

constexpr double radians(double deg)
{
    return deg * (tau / 360);
}

MAPNIK_DECL double normalize_angle(double angle);

}}

#endif
