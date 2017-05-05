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

#ifndef MAPNIK_GEOMETRY_REMOVE_EMPTY_HPP
#define MAPNIK_GEOMETRY_REMOVE_EMPTY_HPP

#include <mapnik/geometry.hpp>
#include <mapnik/geometry/is_empty.hpp>

namespace mapnik { namespace geometry {

namespace detail {

struct geometry_remove_empty
{
    mapnik::geometry::multi_line_string<double> operator() (mapnik::geometry::multi_line_string<double> const& geom) const
    {
        return remove_empty(geom);
    }

    mapnik::geometry::multi_polygon<double> operator() (mapnik::geometry::multi_polygon<double> const& geom) const
    {
        return remove_empty(geom);
    }

    template <typename T>
    T operator() (T const& geom) const
    {
        return geom;
    }

private:
    template <typename T>
    T remove_empty(T const& geom) const
    {
        T new_geom;
        for (auto const & g : geom)
        {
            if (!g.empty())
            {
                new_geom.emplace_back(g);
            }
        }
        return new_geom;
    }
};

}

template <typename GeomType>
inline GeomType remove_empty(GeomType const& geom)
{
    return detail::geometry_remove_empty()(geom);
}

}}

#endif // MAPNIK_GEOMETRY_REMOVE_EMPTY_HPP
