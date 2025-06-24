/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#include <mapnik/geometry.hpp>
#include <mapnik/geometry/boost_adapters.hpp>
#include <boost/geometry/algorithms/centroid.hpp>
#include <mapnik/geometry/is_empty.hpp>
#include <mapnik/geometry/remove_empty.hpp>

namespace mapnik {
namespace geometry {

namespace detail {

template<typename T>
struct geometry_centroid
{
    using result_type = bool;

    geometry_centroid(point<T>& pt)
        : pt_(pt)
    {}

    template<typename U>
    result_type operator()(U const& geom) const
    {
        return util::apply_visitor(*this, geom);
    }

    result_type operator()(geometry_empty const&) const { return false; }

    result_type operator()(geometry_collection<T> const&) const { return false; }

    result_type operator()(point<T> const& geom) const { return centroid_simple(geom); }

    result_type operator()(line_string<T> const& geom) const { return centroid_simple(geom); }

    result_type operator()(polygon<T> const& geom) const { return centroid_simple(geom); }

    result_type operator()(multi_point<T> const& geom) const { return centroid_simple(geom); }

    result_type operator()(multi_line_string<T> const& geom) const { return centroid_multi(geom); }

    result_type operator()(multi_polygon<T> const& geom) const { return centroid_multi(geom); }

    point<T>& pt_;

  private:
    template<typename Geom>
    result_type centroid_simple(Geom const& geom) const
    {
        try
        {
            boost::geometry::centroid(geom, pt_);
            return true;
        }
        catch (boost::geometry::centroid_exception const& e)
        {
            return false;
        }
    }

    template<typename Geom>
    result_type centroid_multi(Geom const& multi_geom) const
    {
        return centroid_simple(multi_geom);
    }
};

} // namespace detail

template<typename T1, typename T2>
inline bool centroid(T1 const& geom, point<T2>& pt)
{
    return detail::geometry_centroid<T2>(pt)(geom);
}

} // namespace geometry
} // namespace mapnik

#endif // MAPNIK_GEOMETRY_CENTROID_HPP
