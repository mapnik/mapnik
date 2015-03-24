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

namespace mapnik { namespace new_geometry {

namespace detail {

struct geometry_empty
{
    bool operator() (mapnik::new_geometry::geometry const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    bool operator() (mapnik::new_geometry::point const&) const
    {
        return false;
    }

    bool operator() (mapnik::new_geometry::line_string const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::new_geometry::polygon const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::new_geometry::multi_point const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::new_geometry::multi_line_string const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::new_geometry::multi_polygon const& geom) const
    {
        return geom.empty();
    }

    bool operator() (mapnik::new_geometry::geometry_collection const& geom) const
    {
        return geom.empty();
    }
};

}

inline bool empty(mapnik::new_geometry::geometry const& geom)
{
    return detail::geometry_empty()(geom);
}

}}

#endif // MAPNIK_GEOMETRY_EMPTY_HPP
