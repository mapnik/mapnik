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

#include <mapnik/geometry/polylabel.hpp>
#include <mapnik/geometry/envelope.hpp>
#include <mapbox/polylabel.hpp>

namespace mapnik { namespace geometry {

template <class T>
T polylabel_precision(polygon<T> const& polygon, double scale_factor)
{
    const box2d<T> bbox = mapnik::geometry::envelope(polygon);
    // Let the precision be 1% of the polygon size to be independent to map scale.
    return (std::max(bbox.width(), bbox.height()) / 100.0) * scale_factor;
}

template <class T>
bool polylabel(polygon<T> const& polygon, T precision, point<T> & pt)
{
    if (polygon.empty() || polygon.front().empty())
    {
        return false;
    }

    pt = mapbox::polylabel(polygon, precision);
    return true;
}

template MAPNIK_DECL 
bool polylabel(polygon<double> const& polygon, 
            double precision, 
            point<double> & pt);

template MAPNIK_DECL 
double polylabel_precision(polygon<double> const& polygon, 
double scale_factor);

} }

