/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#ifndef MAPNIK_GEOMETRY_IS_SIMPLE_HPP
#define MAPNIK_GEOMETRY_IS_SIMPLE_HPP

#include <boost/version.hpp>

// only Boost >= 1.56 contains the is_simple function
#if BOOST_VERSION >= 105600

#include <mapnik/geometry.hpp>
#include <mapnik/geometry/boost_adapters.hpp>
#include <boost/geometry/algorithms/is_simple.hpp>

namespace mapnik {
namespace geometry {

namespace detail {

struct geometry_is_simple
{
    using result_type = bool;

    template<typename T>
    result_type operator()(geometry<T> const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    result_type operator()(geometry_empty const&) const
    {
        // An empty geometry has no anomalous geometric points, such as self intersection or self tangency.
        // Therefore, we will return true
        return true;
    }

    template<typename T>
    result_type operator()(geometry_collection<T> const& collection) const
    {
        for (auto const& geom : collection)
        {
            if (!(*this)(geom))
                return false;
        }
        return true;
    }

    template<typename T>
    result_type operator()(point<T> const& pt) const
    {
        return boost::geometry::is_simple(pt);
    }
    template<typename T>
    result_type operator()(line_string<T> const& line) const
    {
        if (line.empty())
        {
            // Prevent an empty line_string from segfaulting in boost geometry 1.58
            // once it is fixed this can be removed
            // https://svn.boost.org/trac/boost/ticket/11709
            return true;
        }
        return boost::geometry::is_simple(line);
    }
    template<typename T>
    result_type operator()(polygon<T> const& poly) const
    {
        return boost::geometry::is_simple(poly);
    }
    template<typename T>
    result_type operator()(multi_point<T> const& multi_pt) const
    {
        if (multi_pt.empty())
        {
            // This return is due to bug in boost geometry once it is fixed it can be removed
            // https://svn.boost.org/trac/boost/ticket/11710
            return true;
        }
        return boost::geometry::is_simple(multi_pt);
    }
    template<typename T>
    result_type operator()(multi_line_string<T> const& multi_line) const
    {
        if (multi_line.empty())
        {
            // This return is due to bug in boost geometry once it is fixed it can be removed
            // https://svn.boost.org/trac/boost/ticket/11710
            return true;
        }
        for (auto const& line : multi_line)
        {
            if (!(*this)(line))
                return false;
        }
        return true;
    }
    template<typename T>
    result_type operator()(multi_polygon<T> const& multi_poly) const
    {
        if (multi_poly.empty())
        {
            // This return is due to bug in boost geometry once it is fixed it can be removed
            // https://svn.boost.org/trac/boost/ticket/11710
            return true;
        }
        for (auto const& poly : multi_poly)
        {
            if (!(*this)(poly))
                return false;
        }
        return true;
    }
};

} // namespace detail

template<typename T>
inline bool is_simple(T const& geom)
{
    return detail::geometry_is_simple()(geom);
}

template<typename T>
inline bool is_simple(mapnik::geometry::geometry<T> const& geom)
{
    return util::apply_visitor(detail::geometry_is_simple(), geom);
}

} // namespace geometry
} // namespace mapnik

#endif // BOOST_VERSION >= 1.56
#endif // MAPNIK_GEOMETRY_IS_SIMPLE_HPP
