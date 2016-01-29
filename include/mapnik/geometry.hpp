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
#include <mapnik/coord.hpp>
#include <vector>
#include <type_traits>
#include <cstddef>



namespace mapnik { namespace geometry {

template <typename T>
struct point
{
    using value_type = T;
    point() {}
    point(T x_, T y_)
        : x(x_), y(y_)
    {}
    // temp - remove when geometry is templated on value_type
    point(mapnik::coord<double, 2> const& c)
        : x(c.x), y(c.y) {}

    friend inline bool operator== (point<T> const& a, point<T> const& b)
    {
        return a.x == b.x && a.y == b.y;
    }
    friend inline bool operator!= (point<T> const& a, point <T> const& b)
    {
        return a.x != b.x  || a.y != b.y;
    }
    value_type x;
    value_type y;
};


template <typename T>
struct line_string : std::vector<point<T> >
{
    line_string() = default;
    explicit line_string(std::size_t size)
        : std::vector<point<T> >(size) {}
    inline std::size_t num_points() const { return std::vector<point<T>>::size(); }
    inline void add_coord(T x, T y) { std::vector<point<T>>::template emplace_back(x,y);}
};

template <typename T>
struct linear_ring : line_string<T>
{
    linear_ring() = default;
    explicit linear_ring(std::size_t size)
        : line_string<T>(size) {}
    linear_ring(line_string<T> && other)
        : line_string<T>(std::move(other)) {}
    linear_ring(line_string<T> const& other)
        : line_string<T>(other) {}
};

template <typename T>
using rings_container = std::vector<linear_ring<T>>;

template <typename T, template <typename> class InteriorRings = rings_container>
struct polygon
{
    linear_ring<T> exterior_ring;
    using rings_container = InteriorRings<T>;
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

template <typename T>
struct multi_point : line_string<T> {};

template <typename T>
struct multi_line_string : std::vector<line_string<T>> {};

template <typename T>
struct multi_polygon : std::vector<polygon<T>> {};

template <typename T>
struct geometry_collection;

struct geometry_empty {};


template <typename T>
using geometry_base = mapnik::util::variant<geometry_empty,
                                            point<T>,
                                            line_string<T>,
                                            polygon<T>,
                                            multi_point<T>,
                                            multi_line_string<T>,
                                            multi_polygon<T>,
                                            mapnik::util::recursive_wrapper<geometry_collection<T> > >;
template <typename T>
struct geometry : geometry_base<T>
{
    using value_type = T;

    geometry()
        : geometry_base<T>() {} // empty

    template <typename G>
    geometry(G && geom)
        : geometry_base<T>(std::forward<G>(geom)) {}

};

template <typename T>
struct geometry_collection : std::vector<geometry<T>> {};

}}

#endif //MAPNIK_GEOMETRY_HPP
