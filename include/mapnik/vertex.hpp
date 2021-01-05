/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_VERTEX_HPP
#define MAPNIK_VERTEX_HPP

#include <utility>
#include <cstdint>

namespace mapnik
{

enum CommandType : std::uint8_t {
    SEG_END    = 0,
    SEG_MOVETO = 1,
    SEG_LINETO = 2,
    SEG_CLOSE = (0x40 | 0x0f)
};


template <typename T,int dim>
struct vertex {
    using coord_type = T;
};

template <typename T>
struct vertex<T,2>
{
    enum no_init_t : std::uint8_t { no_init };

    using coord_type = T;
    coord_type x;
    coord_type y;
    unsigned cmd;

    vertex()
        : x(0),y(0),cmd(SEG_END) {}

    explicit vertex(no_init_t)
        {}

    vertex(coord_type x_,coord_type y_,unsigned cmd_)
        : x(x_),y(y_),cmd(cmd_) {}

    template <typename T2>
    vertex(vertex<T2,2> const& rhs)
        : x(coord_type(rhs.x)),
          y(coord_type(rhs.y)),
          cmd(rhs.cmd) {}

    template <typename T2>
    vertex<T,2>& operator=(vertex<T2,2> const& rhs)
    {
        vertex<T,2> tmp(rhs);
        swap(tmp);
        return *this;
    }

private:
    void swap(vertex<T,2> & rhs)
    {
        std::swap(this->x,rhs.x);
        std::swap(this->y,rhs.y);
        std::swap(this->cmd,rhs.cmd);
    }
};

using vertex2d = vertex<double,2>;
using vertex2i = vertex<int,2>;

}

#endif // MAPNIK_VERTEX_HPP
