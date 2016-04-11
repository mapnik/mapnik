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

template <typename T>
struct line_string : std::vector<point<T> >
{
    line_string() = default;
    explicit line_string(std::size_t size)
        : std::vector<point<T> >(size) {}
    inline std::size_t num_points() const { return std::vector<point<T>>::size(); }
    inline void add_coord(T x, T y) { std::vector<point<T>>::template emplace_back(x,y);}
};

}}

#endif // MAPNIK_GEOMETRY_LINE_STRING_HPP
