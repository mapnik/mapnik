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

#ifndef MAPNIK_VERTEX_ADAPTERS_HPP
#define MAPNIK_VERTEX_ADAPTERS_HPP

#include <mapnik/geometry.hpp>

namespace mapnik { namespace geometry {

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

    inline geometry_types type () const
    {
        return geometry_types::Point;
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

    inline geometry_types type () const
    {
        return geometry_types::LineString;
    }

    line_string const& line_;
    mutable std::size_t current_index_;
    const std::size_t end_index_;

};

struct polygon_vertex_adapter
{
    polygon_vertex_adapter(polygon const& poly)
        : poly_(poly),
          rings_itr_(0),
          rings_end_(poly_.interior_rings.size() + 1),
          current_index_(0),
          end_index_((rings_itr_ < rings_end_) ? poly_.exterior_ring.size() : 0),
          start_loop_(true),
          close_loop_(false) {}

    void rewind(unsigned) const
    {
        rings_itr_ = 0;
        rings_end_ = poly_.interior_rings.size() + 1;
        current_index_ = 0;
        end_index_ = (rings_itr_ < rings_end_) ? poly_.exterior_ring.size() : 0;
        start_loop_ = true;
        close_loop_ = false;
    }

    unsigned vertex(double* x, double* y) const
    {
        if (rings_itr_ == rings_end_)
        {
            return mapnik::SEG_END;
        }
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
            if (current_index_ == end_index_)
            {
                *x = 0;
                *y = 0;
                return mapnik::SEG_CLOSE;
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

    inline geometry_types type () const
    {
        return geometry_types::Polygon;
    }

private:
    polygon const& poly_;
    mutable std::size_t rings_itr_;
    mutable std::size_t rings_end_;
    mutable std::size_t current_index_;
    mutable std::size_t end_index_;
    mutable bool start_loop_;
    mutable bool close_loop_;
};

template <typename T>
struct vertex_adapter_traits{};

template <>
struct vertex_adapter_traits<point>
{
    using type = point_vertex_adapter;
};

template <>
struct vertex_adapter_traits<line_string>
{
    using type = line_string_vertex_adapter;
};

template <>
struct vertex_adapter_traits<polygon>
{
    using type = polygon_vertex_adapter;
};

template <>
struct vertex_adapter_traits<multi_point>
{
    using type = point_vertex_adapter;
};

template <>
struct vertex_adapter_traits<multi_line_string>
{
    using type = line_string_vertex_adapter;
};

template <>
struct vertex_adapter_traits<multi_polygon>
{
    using type = polygon_vertex_adapter;
};

}}

#endif //MAPNIK_VERTEX_ADAPTERS_HPP
