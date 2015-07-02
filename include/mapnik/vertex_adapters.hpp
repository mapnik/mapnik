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
#include <mapnik/geometry_types.hpp>
#include <mapnik/vertex.hpp>

namespace mapnik { namespace geometry {

template <typename T>
struct point_vertex_adapter
{
    using value_type = typename point<T>::value_type;

    point_vertex_adapter(point<T> const& pt)
        : pt_(pt),
          first_(true) {}

    unsigned vertex(value_type * x, value_type * y) const
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

    point<T> const& pt_;
    mutable bool first_;
};

template <typename T>
struct line_string_vertex_adapter
{
    using value_type = typename point<T>::value_type;
    line_string_vertex_adapter(line_string<T> const& line)
        : line_(line),
          current_index_(0),
          end_index_(line.size())
    {}

    unsigned vertex(value_type * x, value_type * y) const
    {
        if (current_index_ != end_index_)
        {
            point<T> const& coord = line_[current_index_++];
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

    line_string<T> const& line_;
    mutable std::size_t current_index_;
    const std::size_t end_index_;

};

template <typename T>
struct polygon_vertex_adapter
{
    using value_type = typename point<T>::value_type;
    polygon_vertex_adapter(polygon<T> const& poly)
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

    unsigned vertex(value_type * x, value_type * y) const
    {
        if (rings_itr_ == rings_end_)
        {
            return mapnik::SEG_END;
        }
        if (current_index_ < end_index_)
        {
            point<T> const& coord = (rings_itr_ == 0) ?
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
            point<T> const& coord = poly_.interior_rings[rings_itr_ - 1][current_index_++];
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
    polygon<T> const& poly_;
    mutable std::size_t rings_itr_;
    mutable std::size_t rings_end_;
    mutable std::size_t current_index_;
    mutable std::size_t end_index_;
    mutable bool start_loop_;
};

template <typename T>
struct ring_vertex_adapter
{
    using value_type = typename point<T>::value_type;
    ring_vertex_adapter(linear_ring<T> const& ring)
        : ring_(ring),
          current_index_(0),
          end_index_(ring_.size()),
          start_loop_(true) {}

    void rewind(unsigned) const
    {
        current_index_ = 0;
        end_index_ = ring_.size();
        start_loop_ = true;
    }

    unsigned vertex(value_type * x, value_type * y) const
    {
        if (current_index_ < end_index_)
        {
            auto const& coord = ring_[current_index_++];
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
        return mapnik::SEG_END;
    }

    inline geometry_types type () const
    {
        return geometry_types::Polygon;
    }

private:
    linear_ring<T> const& ring_;
    mutable std::size_t current_index_;
    mutable std::size_t end_index_;
    mutable bool start_loop_;
};

template <typename T>
struct vertex_adapter_traits {};

template <>
struct vertex_adapter_traits<point<double> >
{
    using type = point_vertex_adapter<double>;
};

template <>
struct vertex_adapter_traits<line_string<double> >
{
    using type = line_string_vertex_adapter<double>;
};

template <>
struct vertex_adapter_traits<polygon<double> >
{
    using type = polygon_vertex_adapter<double>;
};

template <>
struct vertex_adapter_traits<multi_point<double> >
{
    using type = point_vertex_adapter<double>;
};

template <>
struct vertex_adapter_traits<multi_line_string<double> >
{
    using type = line_string_vertex_adapter<double>;
};

template <>
struct vertex_adapter_traits<multi_polygon<double> >
{
    using type = polygon_vertex_adapter<double>;
};

}}

#endif //MAPNIK_VERTEX_ADAPTERS_HPP
