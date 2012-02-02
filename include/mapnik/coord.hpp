/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

// stl
#include <iomanip>
#include <sstream>

namespace mapnik {
template <typename T,int dim>
struct coord {
    typedef T type;
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
    typedef T type;
    T x;
    T y;
public:
    coord()
        : x(),y() {}
    coord(T x,T y)
        : x(x),y(y) {}
    template <typename T2>
    coord (const coord<T2,2>& rhs)
        : x(type(rhs.x)),
          y(type(rhs.y)) {}

    template <typename T2>
    coord<T,2>& operator=(const coord<T2,2>& rhs)
    {
        if ((void*)this==(void*)&rhs)
        {
            return *this;
        }
        x=type(rhs.x);
        y=type(rhs.y);
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
};

template <typename T>
struct coord<T,3>
{
    typedef T type;
    T x;
    T y;
    T z;
public:
    coord()
        : x(),y(),z() {}
    coord(T x,T y,T z)
        : x(x),y(y),z(z) {}
    template <typename T2>
    coord (const coord<T2,3>& rhs)
        : x(type(rhs.x)),
          y(type(rhs.y)),
          z(type(rhs.z)) {}

    template <typename T2>
    coord<T,3>& operator=(const coord<T2,3>& rhs)
    {
        if ((void*)this==(void*)&rhs)
        {
            return *this;
        }
        x=type(rhs.x);
        y=type(rhs.y);
        z=type(rhs.z);
        return *this;
    }
};

typedef coord<double,2> coord2d;
typedef coord<int,2> coord2i;


template <typename charT,typename traits,typename T ,int dim>
inline std::basic_ostream<charT,traits>&
operator << (std::basic_ostream<charT,traits>& out,
             const coord<T,dim>& c);

template <typename charT,typename traits,typename T>
inline std::basic_ostream<charT,traits>&
operator << (std::basic_ostream<charT,traits>& out,
             const coord<T,2>& c)
{
    std::basic_ostringstream<charT,traits> s;
    s.copyfmt(out);
    s.width(0);
    s << "coord2(" << std::setprecision(16)
      << c.x << "," << c.y<< ")";
    out << s.str();
    return out;
}

template <typename charT,typename traits,typename T>
inline std::basic_ostream<charT,traits>&
operator << (std::basic_ostream<charT,traits>& out,
             const coord<T,3>& c)
{
    std::basic_ostringstream<charT,traits> s;
    s.copyfmt(out);
    s.width(0);
    s << "coord3(" << std::setprecision(16)
      << c.x << "," << c.y<< "," << c.z<<")";
    out << s.str();
    return out;
}
}

#endif // MAPNIK_COORD_HPP
