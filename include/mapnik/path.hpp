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

#ifndef MAPNIK_PATH_HPP
#define MAPNIK_PATH_HPP

// mapnik
#include <mapnik/vertex_vector.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/util/noncopyable.hpp>

namespace mapnik { namespace detail {

template <typename T, template <typename> class Container=vertex_vector>
class path : private util::noncopyable
{
public:
    static const std::uint8_t geometry_bits = 7;
    enum types : std::uint8_t
    {
        Unknown = 0x00,
        Point =   0x01,
        LineString = 0x02,
        Polygon = 0x03,
        PolygonExterior = Polygon,
        PolygonInterior = Polygon | ( 1 << geometry_bits)
    };
    using coordinate_type = T;
    using container_type = Container<coordinate_type>;
    using value_type = typename container_type::value_type;
    using size_type = typename container_type::size_type;
    container_type cont_;
    types type_;
public:

    path()
        : type_(Unknown)
    {}

    explicit path(types type)
        : type_(type)
    {}

    types type() const
    {
        return static_cast<types>(type_ & types::Polygon);
    }

    bool interior() const
    {
        return static_cast<bool>(type_ >> geometry_bits);
    }

    void set_type(types _type)
    {
        type_ = _type;
    }

    container_type const& data() const
    {
        return cont_;
    }

    size_type size() const
    {
        return cont_.size();
    }
    void push_vertex(coordinate_type x, coordinate_type y, CommandType c)
    {
        cont_.push_back(x,y,c);
    }

    void line_to(coordinate_type x,coordinate_type y)
    {
        push_vertex(x,y,SEG_LINETO);
    }

    void move_to(coordinate_type x,coordinate_type y)
    {
        push_vertex(x,y,SEG_MOVETO);
    }

    void close_path()
    {
        push_vertex(0,0,SEG_CLOSE);
    }
};

template <typename T>
struct vertex_adapter : private util::noncopyable
{
    using path_type = T;
    using size_type = typename path_type::size_type;
    using value_type = typename path_type::value_type;
    using types = typename path_type::types;

    vertex_adapter(path_type const& path)
        : path_(path),
          itr_(0) {}

    size_type size() const
    {
        return path_.size();
    }

    unsigned vertex(double* x, double* y) const
    {
        return path_.cont_.get_vertex(static_cast<unsigned>(itr_++),x,y);
    }

    unsigned vertex(std::size_t index, double* x, double* y) const
    {
        return path_.cont_.get_vertex(index, x, y);
    }

    void rewind(unsigned ) const
    {
        itr_ = 0;
    }

    types type() const
    {
        return path_.type();
    }

    box2d<double> envelope() const
    {
        box2d<double> result;
        double x = 0;
        double y = 0;
        rewind(0);
        size_type path_size = size();
        for (size_type i = 0; i < path_size; ++i)
        {
            unsigned cmd = vertex(&x,&y);
            if (cmd == SEG_CLOSE) continue;
            if (i == 0)
            {
                result.init(x,y,x,y);
            }
            else
            {
                result.expand_to_include(x,y);
            }
        }
        return result;
    }
    path_type const& path_;
    mutable size_type itr_;
};
}

template <typename PathType>
box2d<double> envelope(PathType const& path)
{
    detail::vertex_adapter<PathType> va(path);
    return va.envelope();
}

using path_type = detail::path<double,vertex_vector>;
using vertex_adapter = detail::vertex_adapter<path_type>;

}

#endif // MAPNIK_PATH_HPP
