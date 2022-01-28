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

// mapnik
#include <mapnik/util/math.hpp>

// stl
#include <cmath>

namespace mapnik {

namespace util {

double normalize_angle(double angle)
{
    if (angle > pi)
    {
        if (angle > 16 * tau)
        {
            // the angle is too large; better compute the remainder
            // directly to avoid subtracting circles ad infinitum
            return std::remainder(angle, tau);
        }
        // std::remainder would take longer than a few subtractions
        while ((angle -= tau) > pi)
            ;
    }
    else if (angle < -pi)
    {
        if (angle < -16 * tau)
        {
            return std::remainder(angle, tau);
        }
        while ((angle += tau) < -pi)
            ;
    }
    return angle;
}

} // end namespace util

} // end namespace mapnik
