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

#ifndef MAPNIK_GEOMETRY_CENTROID_HPP
#define MAPNIK_GEOMETRY_CENTROID_HPP

#include <mapnik/geometry_impl.hpp>
#include <mapnik/geometry_adapters.hpp>
#include <boost/geometry/algorithms/centroid.hpp>

namespace mapnik { namespace new_geometry {

namespace detail {

struct geometry_centroid
{
    using result_type = bool;

    geometry_centroid(point & pt)
        : pt_(pt) {}

    result_type operator() (geometry const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    result_type operator() (geometry_collection const& collection) const
    {
        return false;
    }

    template <typename T>
    result_type operator() (T const& geom) const
    {
        boost::geometry::centroid(geom, pt_);
        return true;
    }
    point & pt_;
};

}

inline bool centroid(mapnik::new_geometry::geometry const& geom, point & pt)
{
    return detail::geometry_centroid(pt) (geom);
}

}}

#endif // MAPNIK_GEOMETRY_CENTROID_HPP
