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

#ifndef MAPNIK_GEOMETRY_EMPTY_HPP
#define MAPNIK_GEOMETRY_EMPTY_HPP

#include <mapnik/geometry.hpp>

namespace mapnik { namespace geometry {

namespace detail {

struct geometry_is_empty
{
    bool operator() (mapnik::geometry::geometry const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    bool operator() (mapnik::geometry::point const&) const
    {
        return false;
    }

    bool operator() (mapnik::geometry::line_string const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::polygon const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::multi_point const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::multi_line_string const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::multi_polygon const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::geometry::geometry_collection const& geom) const
    {
        return geom.empty();
    }
};

}

inline bool is_empty(mapnik::geometry::geometry const& geom)
{
    return detail::geometry_is_empty()(geom);
}

}}

#endif // MAPNIK_GEOMETRY_EMPTY_HPP
