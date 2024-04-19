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

#ifndef MAPNIK_GEOMETRY_TO_DS_TYPE_HPP
#define MAPNIK_GEOMETRY_TO_DS_TYPE_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/datasource_geometry_type.hpp>
#include <mapnik/util/variant.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/fusion/include/at.hpp>
#include <boost/fusion/include/vector.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {
namespace util {

namespace detail {

struct datasource_geometry_type
{
    mapnik::datasource_geometry_t operator()(mapnik::geometry::geometry_empty const&) const
    {
        return mapnik::datasource_geometry_t::Unknown;
    }

    mapnik::datasource_geometry_t operator()(mapnik::geometry::point<double> const&) const
    {
        return mapnik::datasource_geometry_t::Point;
    }

    mapnik::datasource_geometry_t operator()(mapnik::geometry::line_string<double> const&) const
    {
        return mapnik::datasource_geometry_t::LineString;
    }

    mapnik::datasource_geometry_t operator()(mapnik::geometry::polygon<double> const&) const
    {
        return mapnik::datasource_geometry_t::Polygon;
    }

    mapnik::datasource_geometry_t operator()(mapnik::geometry::multi_point<double> const&) const
    {
        return mapnik::datasource_geometry_t::Point;
    }

    mapnik::datasource_geometry_t operator()(mapnik::geometry::multi_line_string<double> const&) const
    {
        return mapnik::datasource_geometry_t::LineString;
    }

    mapnik::datasource_geometry_t operator()(mapnik::geometry::multi_polygon<double> const&) const
    {
        return mapnik::datasource_geometry_t::Polygon;
    }

    mapnik::datasource_geometry_t operator()(mapnik::geometry::geometry_collection<double> const&) const
    {
        return mapnik::datasource_geometry_t::Collection;
    }
};
} // namespace detail

static inline mapnik::datasource_geometry_t to_ds_type(mapnik::geometry::geometry<double> const& geom)
{
    return util::apply_visitor(detail::datasource_geometry_type(), geom);
}

} // namespace util
} // namespace mapnik

#endif // MAPNIK_GEOMETRY_TO_DS_TYPE_HPP
