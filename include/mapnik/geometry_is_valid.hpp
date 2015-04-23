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

#ifndef MAPNIK_GEOMETRY_IS_VALID_HPP
#define MAPNIK_GEOMETRY_IS_VALID_HPP

#include <boost/version.hpp>

// only Boost >= 1.56 contains the is_valid function
#if BOOST_VERSION >= 105600

#include <mapnik/geometry.hpp>
#include <mapnik/geometry_adapters.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>

namespace mapnik { namespace geometry {

namespace detail {

struct geometry_is_valid
{
    using result_type = bool;

    template <typename T>
    result_type operator() (geometry<T> const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    result_type operator() (geometry_empty const& ) const
    {
        return false;
    }

    template <typename T>
    result_type operator() (geometry_collection<T> const& collection) const
    {
        for (auto const& geom : collection)
        {
            if ( !(*this)(geom)) return false;
        }
        return true;
    }

    template <typename T>
    result_type operator() (point<T> const& pt) const
    {
        return boost::geometry::is_valid(pt);
    }

    template <typename T>
    result_type operator() (line_string<T> const& line) const
    {
        return boost::geometry::is_valid(line);
    }

    template <typename T>
    result_type operator() (polygon<T> const& poly) const
    {
        return boost::geometry::is_valid(poly);
    }

    template <typename T>
    result_type operator() (multi_point<T> const& multi_pt) const
    {
        return boost::geometry::is_valid(multi_pt);
    }

    template <typename T>
    result_type operator() (multi_line_string<T> const& multi_line) const
    {
        return boost::geometry::is_valid(multi_line);
    }

    template <typename T>
    result_type operator() (multi_polygon<T> const& multi_poly) const
    {
        return boost::geometry::is_valid(multi_poly);
    }
};

}

template <typename T>
inline bool is_valid(T const& geom)
{
    return detail::geometry_is_valid() (geom);
}

}}

#endif // BOOST_VERSION >= 1.56
#endif // MAPNIK_GEOMETRY_IS_VALID_HPP
