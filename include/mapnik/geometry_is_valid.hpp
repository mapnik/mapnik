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

#include <mapnik/geometry_impl.hpp>
#include <mapnik/geometry_adapters.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>

namespace mapnik { namespace new_geometry {

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

    template <typename T>
    result_type operator() (T const& geom) const
    {
        return boost::geometry::is_valid(geom);
    }
};

}

inline bool is_valid(mapnik::new_geometry::geometry const& geom)
{
    return detail::geometry_is_valid() (geom);
}

}}

#endif // MAPNIK_GEOMETRY_IS_VALID_HPP
