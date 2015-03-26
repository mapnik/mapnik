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

#ifndef MAPNIK_GEOMETRY_HPP
#define MAPNIK_GEOMETRY_HPP

#include <mapnik/util/variant.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/geometry_types.hpp>

#include <algorithm>
#include <vector>
#include <tuple>
#include <type_traits>
#include <cstddef>
#include <utility>

namespace mapnik { namespace geometry {

struct point
{
    point() {}
    point(double x_, double y_)
        : x(x_), y(y_)
    {}

    point(point const& other) = default;
    point(point && other) noexcept = default;
    point & operator=(point const& other) = default;
    double x;
    double y;
};

struct bounding_box
{
    bounding_box() {} // no-init
    bounding_box(double lox, double loy, double hix, double hiy)
        : p0(lox,loy),
          p1(hix,hiy) {}
    point p0;
    point p1;
};

struct line_string : std::vector<point>
{
    line_string() = default;
    line_string (line_string && other) = default ;
    line_string& operator=(line_string &&) = default;
    line_string (line_string const& ) = default;
    line_string& operator=(line_string const&) = default;
    inline std::size_t num_points() const { return size(); }
    inline void add_coord(double x, double y) { emplace_back(x,y);}
};


struct linear_ring : line_string {};

struct polygon
{
    linear_ring exterior_ring;
    std::vector<linear_ring> interior_rings;

    inline void set_exterior_ring(linear_ring && ring)
    {
        exterior_ring = std::move(ring);
    }

    inline void add_hole(linear_ring && ring)
    {
        interior_rings.emplace_back(std::move(ring));
    }

    inline bool empty() const { return exterior_ring.empty(); }

    inline std::size_t num_rings() const
    {
        return 1 + interior_rings.size();
    }
};

struct multi_point : line_string {};
struct multi_line_string : std::vector<line_string> {};
struct multi_polygon : std::vector<polygon> {};
struct geometry_collection;
struct geometry_empty {};
using geometry = mapnik::util::variant<geometry_empty,
                                       point,
                                       line_string,
                                       polygon,
                                       multi_point,
                                       multi_line_string,
                                       multi_polygon,
                                       mapnik::util::recursive_wrapper<geometry_collection> >;

struct geometry_collection : std::vector<geometry> {};

}}

#endif //MAPNIK_GEOMETRY_HPP
