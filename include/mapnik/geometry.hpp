/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

// mapnik
#include <mapnik/vertex_vector.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/util/noncopyable.hpp>

namespace mapnik {

template <typename T, template <typename> class Container=vertex_vector>
class geometry : private util::noncopyable
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
    using coord_type = T;
    using container_type = Container<coord_type>;
    using value_type = typename container_type::value_type;
    using size_type = typename container_type::size_type;
//private:
    container_type cont_;
    types type_;
public:

    geometry()
        : type_(Unknown)
    {}

    explicit geometry(types type)
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

    void set_type(types type)
    {
        type_ = type;
    }

    container_type const& data() const
    {
        return cont_;
    }

    size_type size() const
    {
        return cont_.size();
    }
    void push_vertex(coord_type x, coord_type y, CommandType c)
    {
        cont_.push_back(x,y,c);
    }

    void line_to(coord_type x,coord_type y)
    {
        push_vertex(x,y,SEG_LINETO);
    }

    void move_to(coord_type x,coord_type y)
    {
        push_vertex(x,y,SEG_MOVETO);
    }

    void close_path()
    {
        push_vertex(0,0,SEG_CLOSE);
    }
};

namespace detail {
template <typename Geometry>
struct vertex_adapter : private util::noncopyable
{
    using size_type = typename Geometry::size_type;
    using value_type = typename Geometry::value_type;
    using types = typename Geometry::types;

    vertex_adapter(Geometry const& geom)
        : geom_(geom),
          itr_(0) {}

    size_type size() const
    {
        return geom_.size();
    }

    unsigned vertex(double* x, double* y) const
    {
        return geom_.cont_.get_vertex(itr_++,x,y);
    }

    unsigned vertex(std::size_t index, double* x, double* y) const
    {
        return geom_.cont_.get_vertex(index, x, y);
    }

    void rewind(unsigned ) const
    {
        itr_ = 0;
    }

    types type() const
    {
        return geom_.type();
    }

    box2d<double> envelope() const
    {
        box2d<double> result;
        double x = 0;
        double y = 0;
        rewind(0);
        size_type geom_size = size();
        for (size_type i = 0; i < geom_size; ++i)
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
    Geometry const& geom_;
    mutable size_type itr_;
};
}

template <typename Geometry>
box2d<double> envelope(Geometry const& geom)
{
    detail::vertex_adapter<Geometry> va(geom);
    return va.envelope();
}

using geometry_type = geometry<double,vertex_vector>;
using vertex_adapter = detail::vertex_adapter<geometry_type>;

}

#endif // MAPNIK_GEOMETRY_HPP
