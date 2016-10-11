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

#ifndef MAPNIK_GEOMETRY_ENVELOPE_HPP
#define MAPNIK_GEOMETRY_ENVELOPE_HPP

#include <mapnik/config.hpp>
#include <mapnik/box2d.hpp>

namespace mapnik {
namespace geometry {

template <typename T>
MAPNIK_DECL auto envelope(T const& geom) -> box2d<typename T::coord_type>;

} // end ns geometry
} // end ns mapnik

#endif // MAPNIK_GEOMETRY_ENVELOPE_HPP
