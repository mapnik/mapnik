/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2016 Artem Pavlenko
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

#ifndef MAPNIK_GEOMETRY_POLYGON_HPP
#define MAPNIK_GEOMETRY_POLYGON_HPP

// geometry
#include <mapbox/geometry/polygon.hpp>

// stl
#include <vector>

namespace mapnik { namespace geometry {

template <typename T>
using linear_ring = mapbox::geometry::linear_ring<T>;

template <typename T>
using rings_container = std::vector<linear_ring<T>>;

template <typename T, template <typename> class InteriorRings = rings_container>
struct polygon
{
    using coordinate_type = T;
    using rings_container = InteriorRings<coordinate_type>;
    linear_ring<T> exterior_ring;
    rings_container interior_rings;

    inline void set_exterior_ring(linear_ring<T> && ring)
    {
        exterior_ring = std::move(ring);
    }

    inline void add_hole(linear_ring<T> && ring)
    {
        interior_rings.emplace_back(std::move(ring));
    }

    inline bool empty() const { return exterior_ring.empty(); }

    inline std::size_t num_rings() const
    {
        return 1 + interior_rings.size();
    }
};

}}

#endif // MAPNIK_GEOMETRY_POLYGON_HPP
