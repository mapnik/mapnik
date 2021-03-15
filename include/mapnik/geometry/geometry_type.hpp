/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_GEOMETRY_TYPE_HPP
#define MAPNIK_GEOMETRY_TYPE_HPP

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/geometry/geometry_types.hpp>

namespace mapnik { namespace geometry { namespace detail {

template <typename T>
struct geometry_type
{
    mapnik::geometry::geometry_types operator () (mapnik::geometry::geometry<T> const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    mapnik::geometry::geometry_types operator() (geometry_empty const& ) const
    {
        return mapnik::geometry::geometry_types::Unknown;
    }

    mapnik::geometry::geometry_types operator () (mapnik::geometry::point<T> const&) const
    {
        return mapnik::geometry::geometry_types::Point;
    }

    mapnik::geometry::geometry_types operator () (mapnik::geometry::line_string<T> const&) const
    {
        return mapnik::geometry::geometry_types::LineString;
    }

    mapnik::geometry::geometry_types operator () (mapnik::geometry::polygon<T> const&) const
    {
        return mapnik::geometry::geometry_types::Polygon;
    }

    mapnik::geometry::geometry_types operator () (mapnik::geometry::multi_point<T> const&) const
    {
        return mapnik::geometry::geometry_types::MultiPoint;
    }

    mapnik::geometry::geometry_types operator () (mapnik::geometry::multi_line_string<T> const&) const
    {
        return mapnik::geometry::geometry_types::MultiLineString;
    }

    mapnik::geometry::geometry_types operator () (mapnik::geometry::multi_polygon<T> const&) const
    {
        return mapnik::geometry::geometry_types::MultiPolygon;
    }

    mapnik::geometry::geometry_types operator () (mapnik::geometry::geometry_collection<T> const&) const
    {
        return mapnik::geometry::geometry_types::GeometryCollection;
    }
};
} // detail

template <typename T>
static inline mapnik::geometry::geometry_types geometry_type(T const& geom)
{
    using coordinate_type = typename T::coordinate_type;
    return detail::geometry_type<coordinate_type>()(geom);
}

}}


#endif // MAPNIK_GEOMETRY_TYPE_HPP
