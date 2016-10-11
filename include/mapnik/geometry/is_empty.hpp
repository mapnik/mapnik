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

struct geometry_is_empty
{
    bool operator() (mapnik::geometry::geometry<double> const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    bool operator() (mapnik::geometry::point<double> const&) const
    {
        return false;
    }

    bool operator() (mapnik::geometry::line_string<double> const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::polygon<double> const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::multi_point<double> const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::multi_line_string<double> const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::multi_polygon<double> const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::geometry_collection<double> const& geom) const
    {
        return geom.empty();
    }

    template <typename T>
    bool operator() (T const&) const
    {
        return true;
    }

};

struct geometry_has_empty
{
    bool operator() (mapnik::geometry::geometry<double> const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    bool operator() (mapnik::geometry::geometry_empty const&) const
    {
        return false;
    }

    bool operator() (mapnik::geometry::point<double> const&) const
    {
        return false;
    }

    bool operator() (mapnik::geometry::line_string<double> const&) const
    {
        return false;
    }

    bool operator() (mapnik::geometry::polygon<double> const&) const
    {
        return false;
    }

    bool operator() (mapnik::geometry::multi_point<double> const&) const
    {
        return false;
    }

    bool operator() (mapnik::geometry::multi_line_string<double> const& geom) const
    {
        return test_multigeometry(geom);
    }

    bool operator() (mapnik::geometry::multi_polygon<double> const& geom) const
    {
        return test_multigeometry(geom);
    }

    bool operator() (mapnik::geometry::geometry_collection<double> const& geom) const
    {
        for (auto const & item : geom)
        {
            if (geometry_is_empty()(item) || (*this)(item))
            {
                return true;
            }
        }
        return false;
    }

    template <typename T>
    bool operator() (T const&) const
    {
        return true;
    }

private:
    template <typename T>
    bool test_multigeometry(T const & geom) const
    {
        for (auto const & item : geom)
        {
            if (item.empty())
            {
                return true;
            }
        }
        return false;
    }
};

}

template <typename GeomType>
inline bool is_empty(GeomType const& geom)
{
    return detail::geometry_is_empty()(geom);
}

template <typename GeomType>
inline bool has_empty(GeomType const& geom)
{
    return detail::geometry_has_empty()(geom);
}

}}

#endif // MAPNIK_GEOMETRY_IS_EMPTY_HPP
