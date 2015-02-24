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

#ifndef MAPNIK_GEOMETRY_IMPL_HPP
#define MAPNIK_GEOMETRY_IMPL_HPP

#include <vector>
#include <mapnik/util/variant.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/util/noncopyable.hpp>

#include <algorithm>
#include <vector>
#include <tuple>
#include <type_traits>
#include <cstddef>
#include <utility>

namespace mapnik { namespace new_geometry {

struct point
{
    point() {}
    point(double x_, double y_)
        : x(x_),y(y_) {}

    point(point const& other)
        : x(other.x),
          y(other.y) {}

    point(point && other) noexcept = default;

    point & operator=(point const& other)
    {
        if (this == &other) return *this;
        point tmp(other);
        std::swap(x, tmp.x);
        std::swap(y, tmp.y);
        return *this;
    }
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
    line_string& operator=(line_string const&) = delete;
    inline std::size_t num_points() const { return size(); }
    inline void add_coord(double x, double y) { emplace_back(x,y);}
};


struct linear_ring : line_string {};

struct polygon3
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

    inline std::size_t num_rings() const
    {
        return 1 + interior_rings.size();
    }
};

struct multi_point : line_string {};
struct multi_line_string : std::vector<line_string> {};
struct multi_polygon : std::vector<polygon3> {};
struct geometry_collection;

typedef mapnik::util::variant<point,
                              line_string,
                              polygon3,
                              multi_point,
                              multi_line_string,
                              multi_polygon,
                              mapnik::util::recursive_wrapper<geometry_collection> > geometry;

struct geometry_collection : std::vector<geometry> {};

struct point_vertex_adapter
{
    point_vertex_adapter(point const& pt)
        : pt_(pt),
          first_(true) {}

    unsigned vertex(double*x, double*y) const
    {
        if (first_)
        {
            *x = pt_.x;
            *y = pt_.y;
            first_ = false;
            return mapnik::SEG_MOVETO;
        }
        return mapnik::SEG_END;
    }

    void rewind(unsigned) const
    {
        first_ = true;
    }
    point const& pt_;
    mutable bool first_;
};

struct line_string_vertex_adapter
{
    line_string_vertex_adapter(line_string const& line)
        : line_(line),
          current_index_(0),
          end_index_(line.size())
    {}

    unsigned vertex(double*x, double*y) const
    {
        if (current_index_ != end_index_)
        {
            point const& coord = line_[current_index_++];
            *x = coord.x;
            *y = coord.y;
            if (current_index_ == 1)
            {
                return mapnik::SEG_MOVETO;
            }
            else
            {
                return mapnik::SEG_LINETO;
            }
        }
        return mapnik::SEG_END;
    }

    void rewind(unsigned) const
    {
        current_index_ = 0;
    }
    line_string const& line_;
    mutable std::size_t current_index_;
    const std::size_t end_index_;

};

struct polygon_vertex_adapter_3
{
    polygon_vertex_adapter_3(polygon3 const& poly)
        : poly_(poly),
          rings_itr_(0),
          rings_end_(poly_.interior_rings.size() + 1),
          current_index_(0),
          end_index_((rings_itr_ < rings_end_) ? poly_.exterior_ring.size() : 0),
          start_loop_(true) {}

    void rewind(unsigned) const
    {
        rings_itr_ = 0;
        rings_end_ = poly_.interior_rings.size() + 1;
        current_index_ = 0;
        end_index_ = (rings_itr_ < rings_end_) ? poly_.exterior_ring.size() : 0;
        start_loop_ = true;
    }

    unsigned vertex(double*x, double*y) const
    {
        if (rings_itr_ == rings_end_)
            return mapnik::SEG_END;
        if (current_index_ < end_index_)
        {
            point const& coord = (rings_itr_ == 0) ?
                poly_.exterior_ring[current_index_++] : poly_.interior_rings[rings_itr_- 1][current_index_++];
            *x = coord.x;
            *y = coord.y;
            if (start_loop_)
            {
                start_loop_= false;
                return mapnik::SEG_MOVETO;
            }
            return mapnik::SEG_LINETO;
        }
        else if (++rings_itr_ != rings_end_)
        {
            current_index_ = 0;
            end_index_ = poly_.interior_rings[rings_itr_ - 1].size();
            point const& coord = poly_.interior_rings[rings_itr_ - 1][current_index_++];
            *x = coord.x;
            *y = coord.y;
            return mapnik::SEG_MOVETO;
        }
        return mapnik::SEG_END;
    }
private:
    polygon3 const& poly_;
    mutable std::size_t rings_itr_;
    mutable std::size_t rings_end_;
    mutable std::size_t current_index_;
    mutable std::size_t end_index_;
    mutable bool start_loop_;
};

//
template <typename T>
struct vertex_processor
{
    using processor_type = T;
    vertex_processor(processor_type const& proc)
        : proc_(proc) {}

    auto operator() (point const& pt) const
        -> typename std::result_of<processor_type(point_vertex_adapter const&)>::type
    {
        point_vertex_adapter va(pt);
        return proc_(va);
    }

    auto operator() (line_string const& line)
        -> typename std::result_of<processor_type(line_string_vertex_adapter const&)>::type
    {
        line_string_vertex_adapter va(line);
        return proc_(va);
    }

    auto operator() (polygon3 const& poly) const
        -> typename std::result_of<processor_type(polygon_vertex_adapter_3 const&)>::type
    {
        polygon_vertex_adapter_3 va(poly);
        return proc_(va);
    }

    processor_type const& proc_;
};

}}

#endif //MAPNIK_GEOMETRY_IMPL_HPP
