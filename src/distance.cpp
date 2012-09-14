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

// mapnik
#include <mapnik/distance.hpp>
#include <mapnik/ellipsoid.hpp>

// stl
#include <cmath>

namespace mapnik {

using std::atan2;
using std::cos;
using std::pow;
using std::sin;
using std::sqrt;

static const double deg2rad = 0.0174532925199432958;
static const double R = 6372795.0; // average great-circle radius of the earth

double great_circle_distance::operator() (coord2d const& pt0,
                                          coord2d const& pt1) const
{
    double lon0 = pt0.x * deg2rad;
    double lat0 = pt0.y * deg2rad;
    double lon1 = pt1.x * deg2rad;
    double lat1 = pt1.y * deg2rad;

    double dlat = lat1 - lat0;
    double dlon = lon1 - lon0;

    double sin_dlat = sin(0.5 * dlat);
    double sin_dlon = sin(0.5 * dlon);

    double a = pow(sin_dlat,2.0) + cos(lat0)*cos(lat1)*pow(sin_dlon,2.0);
    double c = 2 * atan2(sqrt(a),sqrt(1 - a));
    return R * c;
}
}
