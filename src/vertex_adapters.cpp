/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#include <mapnik/vertex_adapters.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry/geometry_types.hpp>
#include <mapnik/vertex.hpp>

namespace mapnik { namespace geometry {

// point adapter
template <typename T>
point_vertex_adapter<T>::point_vertex_adapter(point<T> const& pt)
    : pt_(pt),
      first_(true) {}

template <typename T>
unsigned point_vertex_adapter<T>::vertex(coordinate_type * x, coordinate_type * y) const
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

template <typename T>
void point_vertex_adapter<T>::rewind(unsigned) const
{
    first_ = true;
}

template <typename T>
geometry_types point_vertex_adapter<T>::type () const
{
    return geometry_types::Point;
}

// line_string adapter
template <typename T>
line_string_vertex_adapter<T>::line_string_vertex_adapter(line_string<T> const& line)
    : line_(line),
      current_index_(0),
      end_index_(line.size())
{}

template <typename T>
unsigned line_string_vertex_adapter<T>::vertex(coordinate_type * x, coordinate_type * y) const
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

template <typename T>
void line_string_vertex_adapter<T>::rewind(unsigned) const
{
    current_index_ = 0;
}

template <typename T>
geometry_types line_string_vertex_adapter<T>::type() const
{
    return geometry_types::LineString;
}

template <typename T>
polygon_vertex_adapter<T>::polygon_vertex_adapter(polygon<T> const& poly)
    : poly_(poly),
      rings_itr_(0),
      rings_end_(poly_.size()),
      current_index_(0),
      end_index_(poly_.empty() ? 0 : poly_[0].size()),
      start_loop_(true) {}

template <typename T>
void polygon_vertex_adapter<T>::rewind(unsigned) const
{
    rings_itr_ = 0;
    rings_end_ = poly_.size();
    current_index_ = 0;
    end_index_ = poly_.empty() ? 0 : poly_[0].size();
    start_loop_ = true;
}
template <typename T>
unsigned polygon_vertex_adapter<T>::vertex(coordinate_type * x, coordinate_type * y) const
{
    if (rings_itr_ == rings_end_)
    {
        return mapnik::SEG_END;
    }
    if (current_index_ < end_index_)
    {
        point<T> const& coord = poly_[rings_itr_][current_index_++];
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
        end_index_ = poly_[rings_itr_].size();
        if (end_index_ == 0)
        {
            *x = 0;
            *y = 0;
            return mapnik::SEG_CLOSE;
        }
        point<T> const& coord = poly_[rings_itr_][current_index_++];
        *x = coord.x;
        *y = coord.y;
        return mapnik::SEG_MOVETO;
    }
    return mapnik::SEG_END;
}

template <typename T>
geometry_types polygon_vertex_adapter<T>::type () const
{
    return geometry_types::Polygon;
}

// ring adapter
template <typename T>
ring_vertex_adapter<T>::ring_vertex_adapter(linear_ring<T> const& ring)
    : ring_(ring),
      current_index_(0),
      end_index_(ring_.size()),
      start_loop_(true) {}

template <typename T>
void ring_vertex_adapter<T>::rewind(unsigned) const
{
    current_index_ = 0;
    end_index_ = ring_.size();
    start_loop_ = true;
}

template <typename T>
unsigned ring_vertex_adapter<T>::vertex(coordinate_type * x, coordinate_type * y) const
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
template <typename T>
geometry_types ring_vertex_adapter<T>::type () const
{
    return geometry_types::Polygon;
}

template struct point_vertex_adapter<double>;
template struct line_string_vertex_adapter<double>;
template struct polygon_vertex_adapter<double>;
template struct ring_vertex_adapter<double>;

}}
