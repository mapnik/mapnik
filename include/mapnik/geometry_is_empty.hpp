/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_GEOMETRY_IS_EMPTY_HPP
#define MAPNIK_GEOMETRY_IS_EMPTY_HPP

#include <mapnik/geometry.hpp>

namespace mapnik { namespace geometry {

namespace detail {

template <typename T>
struct geometry_is_empty
{
    bool operator() (mapnik::geometry::geometry<T> const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    bool operator() (mapnik::geometry::point<T> const&) const
    {
        return false;
    }

    bool operator() (mapnik::geometry::line_string<T> const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::linear_ring<T> const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::polygon<T> const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::multi_point<T> const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::multi_line_string<T> const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::multi_polygon<T> const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::geometry_collection<T> const& geom) const
    {
        return geom.empty();
    }

    template <typename U>
    bool operator() (U const&) const
    {
        return true;
    }

};

template <typename T>
struct geometry_has_empty
{
    bool operator() (mapnik::geometry::geometry<T> const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    bool operator() (mapnik::geometry::geometry_empty<T> const&) const
    {
        return false;
    }

    bool operator() (mapnik::geometry::point<T> const&) const
    {
        return false;
    }

    bool operator() (mapnik::geometry::line_string<T> const&) const
    {
        return false;
    }

    bool operator() (mapnik::geometry::linear_ring<T> const&) const
    {
        return false;
    }

    bool operator() (mapnik::geometry::polygon<T> const&) const
    {
        return false;
    }

    bool operator() (mapnik::geometry::multi_point<T> const&) const
    {
        return false;
    }

    bool operator() (mapnik::geometry::multi_line_string<T> const& multi_line) const
    {
        for (auto const& line : multi_line)
        {
            if (line.empty()) return true;
        }
        return false;
    }

    bool operator() (mapnik::geometry::multi_polygon<T> const& multi_poly) const
    {
        for (auto const& poly : multi_poly)
        {
            if (poly.empty() || poly.front().empty()) // no-rings OR exterioir is empty
            {
                return true;
            }
        }
        return false;
    }

    bool operator() (mapnik::geometry::geometry_collection<T> const& geom) const
    {
        for (auto const & item : geom)
        {
            if (geometry_is_empty<T>()(item) || (*this)(item))
            {
                return true;
            }
        }
        return false;
    }

    template <typename U>
    bool operator() (U const&) const
    {
        return true;
    }
};

}

template <typename G>
inline bool is_empty(G const& geom)
{
    using coordinate_type = typename G::coordinate_type;
    return detail::geometry_is_empty<coordinate_type>()(geom);
}

template <typename G>
inline bool has_empty(G const& geom)
{
    using coordinate_type = typename G::coordinate_type;
    return detail::geometry_has_empty<coordinate_type>()(geom);
}

}}

#endif // MAPNIK_GEOMETRY_IS_EMPTY_HPP
