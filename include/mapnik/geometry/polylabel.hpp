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

#ifndef MAPNIK_GEOMETRY_POLYLABEL_HPP
#define MAPNIK_GEOMETRY_POLYLABEL_HPP

#include <mapnik/config.hpp> // for MAPNIK_DECL
#include <mapnik/geometry/polygon.hpp>
#include <mapnik/geometry/point.hpp>

namespace mapnik { namespace geometry {

template <class T>
MAPNIK_DECL bool polylabel(polygon<T> const& polygon,
                           T precision,
                           point<T> & pt);

template <class T>
MAPNIK_DECL T polylabel_precision(polygon<T> const& polygon,
                                  double scale_factor);

} }

#endif // MAPNIK_GEOMETRY_POLYLABEL_HPP
