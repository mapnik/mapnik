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

#ifndef MAPNIK_GEOMETRY_POLYGON_HPP
#define MAPNIK_GEOMETRY_POLYGON_HPP

// geometry
#include <mapbox/geometry/polygon.hpp>

namespace mapnik { namespace geometry {

template <typename T>
using linear_ring = mapbox::geometry::linear_ring<T>;

template <typename T>
struct polygon : mapbox::geometry::polygon<T>
{
    using coordinate_type = T;
    using base_type = mapbox::geometry::polygon<T>;
    using linear_ring_type = linear_ring<T>;
    struct interior_rings
    {
        using iterator = typename base_type::iterator;
        using const_iterator = typename base_type::const_iterator;
        using value_type = typename base_type::value_type;
        interior_rings(polygon<coordinate_type> & poly)
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

        void push_back(value_type const& val) { poly_.push_back(val); }
        value_type& back() { return poly_.back(); }
        value_type const& back()  const { return poly_.back(); }
        polygon<coordinate_type> & poly_;
    };

    polygon()
        : base_type(),
          interior_(*this)
    {
        //this->resize(1); // explicit exterior ring ?
    }

    polygon(polygon && other)
        : base_type(std::move(other)),
          interior_(*this) {}

    polygon(polygon const& other)
        : base_type(other),
          interior_(*this) {}

    interior_rings const& interior() const
    {
        return interior_;
    }
    interior_rings & interior()
    {
        return interior_;
    }
    interior_rings interior_;
};

}}

#endif // MAPNIK_GEOMETRY_POLYGON_HPP
