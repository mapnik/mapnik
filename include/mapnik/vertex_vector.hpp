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
//  Credits:
//  I gratefully acknowledge the inspiring work of Maxim Shemanarev (McSeem),
//  author of Anti-Grain Geometry (http://www.antigrain.com). I have used
//  the datastructure from AGG as a template for my own.

#ifndef MAPNIK_VERTEX_VECTOR_HPP
#define MAPNIK_VERTEX_VECTOR_HPP

// mapnik
#include <mapnik/vertex.hpp>

// boost
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>

#include <cstring>  // required for memcpy with linux/g++
#include <vector>

namespace mapnik
{

template <typename T>
class vertex_vector : private boost::noncopyable
{
    typedef T coord_type;
    typedef vertex<coord_type,2> vertex_type;

public:
    // required for iterators support
    typedef boost::tuple<unsigned,coord_type,coord_type> value_type;
    typedef std::size_t size_type;
    typedef std::vector<value_type> value_type_vector;

private:
    value_type_vector vertices_;

public:

    vertex_vector() {}

    ~vertex_vector() {}

    size_type size() const
    {
        return vertices_.size();
    }

    void push_back (coord_type x,coord_type y,unsigned command)
    {
        vertices_.push_back(boost::make_tuple(command,x,y));
    }

    unsigned get_vertex(unsigned pos,coord_type* x,coord_type* y) const
    {
        if (pos >= size()) return SEG_END;
        const value_type& v = vertices_[pos];
        *x = boost::get<1>(v);
        *y = boost::get<2>(v);
        return boost::get<0>(v);
    }

    void set_command(unsigned pos, unsigned command)
    {
        if (pos < size())
        {
            boost::get<0>(vertices_[pos]) = command;
        }
    }
private:
};

}

#endif // MAPNIK_VERTEX_VECTOR_HPP
