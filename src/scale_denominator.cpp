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

// mapnik
#include <mapnik/scale_denominator.hpp>
#include <mapnik/global.hpp>
#include <mapnik/well_known_srs.hpp>

// stl
#include <cmath>

namespace mapnik {

static double const meters_per_degree = EARTH_CIRCUMFERENCE / 360;

double scale_denominator(double map_scale, bool geographic)
{
    // Pixel size in meters.
    // Corresponding approximate resolution is 90.71 DPI.
    constexpr double pixel_size = 0.00028;

    double denom = map_scale / pixel_size;
    if (geographic)
        denom *= meters_per_degree;
    return denom;
}
} // namespace mapnik
