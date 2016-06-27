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

#ifndef MAPNIK_GEOMETRY_LINE_STRING_HPP
#define MAPNIK_GEOMETRY_LINE_STRING_HPP

// mapnik
#include <mapnik/geometry/point.hpp>
// stl
#include <vector>

namespace mapnik { namespace geometry {

template <typename T, template <typename...> class Cont = std::vector>
struct line_string : Cont<point<T> >
{
    using coordinate_type = T;
    using point_type = point<coordinate_type>;
    using container_type = Cont<point_type>;
    line_string() = default;
    explicit line_string(std::size_t size)
        : container_type(size) {}
    inline std::size_t num_points() const { return container_type::size(); }
    inline void add_coord(coordinate_type x, coordinate_type y) { container_type::template emplace_back(x,y);}
};

}}

#endif // MAPNIK_GEOMETRY_LINE_STRING_HPP
