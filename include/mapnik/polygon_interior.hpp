/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2016 Artem Pavlenko
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

#ifndef MAPNIK_POLYGON_INTERIOR_HPP
#define MAPNIK_POLYGON_INTERIOR_HPP

#include <mapnik/geometry/polygon.hpp>


namespace mapnik { namespace detail {

template <typename T>
struct polygon_interior
{
    using coordinate_type = T;
    using iterator = typename geometry::polygon<coordinate_type>::iterator;
    using const_iterator = typename geometry::polygon<coordinate_type>::const_iterator;
    using value_type = typename geometry::polygon<coordinate_type>::value_type;

    polygon_interior(geometry::polygon<coordinate_type> & poly)
        : poly_(poly) {}

    iterator begin()
    {
        auto itr = poly_.begin();
        std::advance(itr, 1);
        return itr;
    }

    iterator end() { return poly_.end();}
    const_iterator begin() const
    {
        auto itr = poly_.begin();
        std::advance(itr, 1);
        return itr;
    }
    const_iterator end() const { return poly_.end();}

    void clear()
    {
        poly_.resize(1);
    }
    void resize(std::size_t size)
    {
        poly_.resize(size + 1);
    }

    std::size_t size() const
    {
        return poly_.empty() ? 0 : poly_.size() - 1;
    }

    value_type& back() { return poly_.back(); }
    value_type const& back()  const { return poly_.back(); }
    geometry::polygon<coordinate_type> & poly_;
};

}}

#endif // MAPNIK_POLYGON_INTERIOR_HPP
