/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_GEOMETRY_REPROJECTION_HPP
#define MAPNIK_GEOMETRY_REPROJECTION_HPP

// mapnik
#include <mapnik/proj_transform.hpp>
#include <mapnik/geometry.hpp>


namespace mapnik {

namespace geometry {

// There is a difference between reprojecting a const vs reprojecting a non const in behavior. If you reproject a
// const a new geometry (or perhaps empty geometry) will be returned no matter how many geometries fail to reproject.
// This is done this way so that large geometry collections that only have a few failing points or polygon parts could
// still be return with out the few failing projections.

template <typename T>
MAPNIK_DECL geometry<T> reproject_copy(geometry<T> const& geom, proj_transform const& proj_trans, unsigned int & n_err);

template <typename T>
MAPNIK_DECL T reproject_copy(T const& geom, proj_transform const& proj_trans, unsigned int & n_err);

template <typename T>
MAPNIK_DECL T reproject_copy(T const& geom, projection const& source, projection const& dest, unsigned int & n_err);


// No error count is required for a non const reprojection and this will reproject in place.
// because the reprojection is done on the same memory it is important to check if it succeeded,
// otherwise you could be dealing with a corrupt geometry.

template <typename T>
MAPNIK_DECL bool reproject(T & geom, proj_transform const& proj_trans);

template <typename T>
MAPNIK_DECL bool reproject(T & geom, projection const& source, projection const& dest);

} // end geometry ns

} // end mapnik ns

#endif // MAPNIK_GEOMETRY_REPROJECTION_HPP
