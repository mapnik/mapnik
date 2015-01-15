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

#ifndef MAPNIK_COORD_HPP
#define MAPNIK_COORD_HPP

// boost
#include <boost/operators.hpp>

namespace mapnik {
template <typename T,int dim>
struct coord {
    using type = T;
};

template <typename T>
struct coord<T,2>
    : boost::addable<coord<T,2>,
                     boost::addable2<coord<T,2>,T,
                                     boost::subtractable<coord<T,2>,
                                                         boost::subtractable2<coord<T,2>,T,
                                                                              boost::dividable2<coord<T,2>, T,
                                                                                                boost::multipliable2<coord<T,2>, T > > > > > >

{
    using type = T;
    T x;
    T y;
public:
    coord()
        : x(),y() {}
    coord(T x_,T y_)
        : x(x_),y(y_) {}

    coord(coord<T,2> const& rhs)
        : x(rhs.x),
          y(rhs.y) {}

    template <typename T2>
    coord (coord<T2,2> const& rhs)
        : x(type(rhs.x)),
          y(type(rhs.y)) {}

    coord(coord<T,2> && rhs) noexcept
        : x(std::move(rhs.x)),
          y(std::move(rhs.y)) {}

    coord<T,2>& operator=(coord<T,2> rhs)
    {
        swap(rhs);
        return *this;
    }

    template <typename T2>
    coord<T,2>& operator=(const coord<T2,2>& rhs)
    {
        coord<T,2> tmp(rhs);
        swap(rhs);
        return *this;
    }

    template <typename T2>
    bool operator==(coord<T2,2> const& rhs)
    {
        return x == rhs.x && y == rhs.y;
    }

    coord<T,2>& operator+=(coord<T,2> const& rhs)
    {
        x+=rhs.x;
        y+=rhs.y;
        return *this;
    }

    coord<T,2>& operator+=(T rhs)
    {
        x+=rhs;
        y+=rhs;
        return *this;
    }

    coord<T,2>& operator-=(coord<T,2> const& rhs)
    {
        x-=rhs.x;
        y-=rhs.y;
        return *this;
    }

    coord<T,2>& operator-=(T rhs)
    {
        x-=rhs;
        y-=rhs;
        return *this;
    }

    coord<T,2>& operator*=(T t)
    {
        x*=t;
        y*=t;
        return *this;
    }
    coord<T,2>& operator/=(T t)
    {
        x/=t;
        y/=t;
        return *this;
    }
private:
    void swap(coord<T,2> & rhs)
    {
        std::swap(this->x, rhs.x);
        std::swap(this->y, rhs.y);
    }
};

template <typename T>
struct coord<T,3>
{
    using type = T;
    T x;
    T y;
    T z;
public:
    coord()
        : x(),y(),z() {}
    coord(T x_,T y_,T z_)
        : x(x_),y(y_),z(z_) {}

    template <typename T2>
    coord (coord<T2,3> const& rhs)
        : x(type(rhs.x)),
          y(type(rhs.y)),
          z(type(rhs.z)) {}

    coord(coord<T,3> && rhs) noexcept
        : x(std::move(rhs.x)),
          y(std::move(rhs.y)),
          z(std::move(rhs.z)) {}

    coord<T,3> operator=(coord<T,3> rhs)
    {
        swap(rhs);
        return *this;
    }

    template <typename T2>
    coord<T,3>& operator=(coord<T2,3> const& rhs)
    {
        coord<T,3> tmp(rhs);
        swap(tmp);
        return *this;
    }
private:
    void swap(coord<T,3> & rhs)
    {
        std::swap(this->x, rhs.x);
        std::swap(this->y, rhs.y);
        std::swap(this->z, rhs.z);
    }
};

using coord2d = coord<double,2>;
using coord2i = coord<int,2>;

}

#endif // MAPNIK_COORD_HPP
