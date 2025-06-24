/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_VERTEX_ADAPTERS_HPP
#define MAPNIK_VERTEX_ADAPTERS_HPP

#include <mapnik/geometry.hpp>
#include <mapnik/geometry/geometry_types.hpp>
#include <mapnik/vertex.hpp>

namespace mapnik {
namespace geometry {

template<typename T>
struct point_vertex_adapter
{
    using coordinate_type = T;
    point_vertex_adapter(point<T> const& pt);
    unsigned vertex(coordinate_type* x, coordinate_type* y) const;
    void rewind(unsigned) const;
    geometry_types type() const;
    point<T> const& pt_;
    mutable bool first_;
};

template<typename T>
struct line_string_vertex_adapter
{
    using coordinate_type = T;
    line_string_vertex_adapter(line_string<T> const& line);
    unsigned vertex(coordinate_type* x, coordinate_type* y) const;
    void rewind(unsigned) const;
    geometry_types type() const;
    line_string<T> const& line_;
    mutable std::size_t current_index_;
    const std::size_t end_index_;
};

template<typename T>
struct polygon_vertex_adapter
{
    using coordinate_type = T;
    polygon_vertex_adapter(polygon<T> const& poly);
    void rewind(unsigned) const;
    unsigned vertex(coordinate_type* x, coordinate_type* y) const;
    geometry_types type() const;

  private:
    polygon<T> const& poly_;
    mutable std::size_t rings_itr_;
    mutable std::size_t rings_end_;
    mutable std::size_t current_index_;
    mutable std::size_t end_index_;
    mutable bool start_loop_;
};

template<typename T>
struct ring_vertex_adapter
{
    using coordinate_type = T;
    ring_vertex_adapter(linear_ring<T> const& ring);
    void rewind(unsigned) const;
    unsigned vertex(coordinate_type* x, coordinate_type* y) const;
    geometry_types type() const;

  private:
    linear_ring<T> const& ring_;
    mutable std::size_t current_index_;
    mutable std::size_t end_index_;
    mutable bool start_loop_;
};

extern template struct MAPNIK_DECL point_vertex_adapter<double>;
extern template struct MAPNIK_DECL line_string_vertex_adapter<double>;
extern template struct MAPNIK_DECL polygon_vertex_adapter<double>;
extern template struct MAPNIK_DECL ring_vertex_adapter<double>;

template<typename T>
struct vertex_adapter_traits
{};

template<>
struct vertex_adapter_traits<point<double>>
{
    using type = point_vertex_adapter<double>;
};

template<>
struct vertex_adapter_traits<line_string<double>>
{
    using type = line_string_vertex_adapter<double>;
};

template<>
struct vertex_adapter_traits<polygon<double>>
{
    using type = polygon_vertex_adapter<double>;
};

template<>
struct vertex_adapter_traits<multi_point<double>>
{
    using type = point_vertex_adapter<double>;
};

template<>
struct vertex_adapter_traits<multi_line_string<double>>
{
    using type = line_string_vertex_adapter<double>;
};

template<>
struct vertex_adapter_traits<multi_polygon<double>>
{
    using type = polygon_vertex_adapter<double>;
};

} // namespace geometry
} // namespace mapnik

#endif // MAPNIK_VERTEX_ADAPTERS_HPP
