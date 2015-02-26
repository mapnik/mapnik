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

#ifndef MAPNIK_GEOMETRY_TYPE_HPP
#define MAPNIK_GEOMETRY_TYPE_HPP

// mapnik
#include <mapnik/geometry_impl.hpp>
#include <mapnik/geometry_types.hpp>

namespace mapnik { namespace new_geometry { namespace detail {

struct geometry_type
{
    mapnik::new_geometry::geometry_types operator () (mapnik::new_geometry::geometry const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    mapnik::new_geometry::geometry_types operator () (mapnik::new_geometry::point const&) const
    {
        return mapnik::new_geometry::geometry_types::Point;
    }

    mapnik::new_geometry::geometry_types operator () (mapnik::new_geometry::line_string const&) const
    {
        return mapnik::new_geometry::geometry_types::LineString;
    }

    mapnik::new_geometry::geometry_types operator () (mapnik::new_geometry::polygon const&) const
    {
        return mapnik::new_geometry::geometry_types::Polygon;
    }

    mapnik::new_geometry::geometry_types operator () (mapnik::new_geometry::multi_point const&) const
    {
        return mapnik::new_geometry::geometry_types::MultiPoint;
    }

    mapnik::new_geometry::geometry_types operator () (mapnik::new_geometry::multi_line_string const&) const
    {
        return mapnik::new_geometry::geometry_types::MultiLineString;
    }

    mapnik::new_geometry::geometry_types operator () (mapnik::new_geometry::multi_polygon const&) const
    {
        return mapnik::new_geometry::geometry_types::MultiPolygon;
    }

    mapnik::new_geometry::geometry_types operator () (mapnik::new_geometry::geometry_collection const&) const
    {
        return mapnik::new_geometry::geometry_types::GeometryCollection;
    }
};
} // detail

static inline mapnik::new_geometry::geometry_types geometry_type(mapnik::new_geometry::geometry const& geom)
{
    return detail::geometry_type()(geom);
}

}}


#endif // MAPNIK_GEOMETRY_TYPE_HPP
