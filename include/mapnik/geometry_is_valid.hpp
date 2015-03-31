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

#include <mapnik/geometry.hpp>
#include <mapnik/geometry_adapters.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>

namespace mapnik { namespace geometry {

namespace detail {

struct geometry_is_valid
{
    using result_type = bool;

    result_type operator() (geometry const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    result_type operator() (geometry_empty const& ) const
    {
        return false;
    }

    result_type operator() (geometry_collection const& collection) const
    {
        for (auto const& geom : collection)
        {
            if ( !(*this)(geom)) return false;
        }
        return true;
    }

    result_type operator() (point const& pt) const
    {
        return boost::geometry::is_valid(pt);
    }

    result_type operator() (line_string const& line) const
    {
        return boost::geometry::is_valid(line);
    }

    result_type operator() (polygon const& poly) const
    {
        return boost::geometry::is_valid(poly);
    }

    result_type operator() (multi_point const& multi_pt) const
    {
        return boost::geometry::is_valid(multi_pt);
    }

    result_type operator() (multi_line_string const& multi_line) const
    {
        return boost::geometry::is_valid(multi_line);
    }

    result_type operator() (multi_polygon const& multi_poly) const
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

#endif // MAPNIK_GEOMETRY_IS_VALID_HPP
