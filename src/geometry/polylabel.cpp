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

#include <mapnik/geometry/polylabel.hpp>
#include <mapnik/geometry/envelope.hpp>
#include <mapbox/polylabel.hpp>

namespace mapnik { namespace geometry {

template <class T>
T polylabel_precision(polygon<T> const& polygon)
{
    return 1;
}

template <class T>
point<T> polylabel(polygon<T> const& polygon, T precision)
{
    return mapbox::polylabel(polygon, precision);
}

template
point<double> polylabel(polygon<double> const& polygon, double precision);

template
double polylabel_precision(polygon<double> const& polygon);

} }

