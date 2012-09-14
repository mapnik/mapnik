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

#ifndef MAPNIK_VERTEX_HPP
#define MAPNIK_VERTEX_HPP

#include <sstream>

namespace mapnik
{

enum CommandType {
    SEG_END    = 0,
    SEG_MOVETO = 1,
    SEG_LINETO = 2,
    SEG_CLOSE = (0x40 | 0x0f)
};

template <typename T,int dim>
struct vertex {
    typedef T coord_type;
};

template <typename T>
struct vertex<T,2>
{
    enum no_init_t { no_init };

    typedef T coord_type;
    coord_type x;
    coord_type y;
    unsigned cmd;

    vertex()
        : x(0),y(0),cmd(SEG_END) {}

    explicit vertex(no_init_t)
        {}

    vertex(coord_type x,coord_type y,unsigned cmd)
        : x(x),y(y),cmd(cmd) {}

    template <typename T2>
    vertex(const vertex<T2,2>& rhs)
        : x(coord_type(rhs.x)),
          y(coord_type(rhs.y)),
          cmd(rhs.cmd) {}

    template <typename T2> vertex<T,2> operator=(const vertex<T2,2>& rhs)
    {
        if (&cmd != &rhs.cmd)
        {
            x = coord_type(rhs.x);
            y = coord_type(rhs.y);
            cmd = rhs.cmd;
        }
        return *this;
    }
};

typedef vertex<double,2> vertex2d;
typedef vertex<int,2> vertex2i;

template <class charT,class traits,class T,int dim>
inline std::basic_ostream<charT,traits>&
operator << (std::basic_ostream<charT,traits>& out,
             const vertex<T,dim>& c);

template <class charT,class traits,class T>
inline std::basic_ostream<charT,traits>&
operator << (std::basic_ostream<charT,traits>& out,
             const vertex<T,2>& v)
{
    std::basic_ostringstream<charT,traits> s;
    s.copyfmt(out);
    s.width(0);
    switch (v.cmd)
    {
        case SEG_END: s << "End "; break;
        case SEG_MOVETO: s << "MoveTo "; break;
        case SEG_LINETO: s << "LineTo "; break;
        case SEG_CLOSE: s << "Close "; break;
    }
    s << "(" << v.x << ", " << v.y << ")";
    return out << s.str();
}

template <class charT,class traits,class T>
inline std::basic_ostream<charT,traits>&
operator << (std::basic_ostream<charT,traits>& out,
             const vertex<T,3>& v)
{
    std::basic_ostringstream<charT,traits> s;
    s.copyfmt(out);
    s.width(0);
    s<<"vertex3("<<v.x<<","<<v.y<<","<<v.z<<",cmd="<<v.cmd<<")";
    out << s.str();
    return out;
}

}

#endif // MAPNIK_VERTEX_HPP
