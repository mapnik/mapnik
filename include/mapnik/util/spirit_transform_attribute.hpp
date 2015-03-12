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

#ifndef MAPNIK_UTIL_SPIRIT_TRANSFORM_ATTRIBUTE_HPP
#define MAPNIK_UTIL_SPIRIT_TRANSFORM_ATTRIBUTE_HPP

#include <mapnik/geometry_impl.hpp>

namespace boost { namespace spirit { namespace traits {

    template <>
    struct transform_attribute<mapnik::new_geometry::geometry const,
                               mapnik::new_geometry::point const&, karma::domain>
    {
        using type = mapnik::new_geometry::point const&;
        static type pre(mapnik::new_geometry::geometry const& geom)
        {
            return mapnik::util::get<mapnik::new_geometry::point>(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::new_geometry::geometry const,
                               mapnik::new_geometry::line_string const&, karma::domain>
    {
        using type = mapnik::new_geometry::line_string const&;
        static type pre(mapnik::new_geometry::geometry const& geom)
        {
            return mapnik::util::get<mapnik::new_geometry::line_string>(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::new_geometry::geometry const,
                               mapnik::new_geometry::polygon const&, karma::domain>
    {
        using type = mapnik::new_geometry::polygon const&;
        static type pre(mapnik::new_geometry::geometry const& geom)
        {
            return mapnik::util::get<mapnik::new_geometry::polygon>(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::new_geometry::polygon const,
                               std::vector<mapnik::new_geometry::linear_ring> const&, karma::domain>
    {
        using type = std::vector<mapnik::new_geometry::linear_ring> const&;
        static type pre(mapnik::new_geometry::polygon const& poly)
        {
            return poly.interior_rings;
        }
    };

    template <>
    struct transform_attribute<mapnik::new_geometry::geometry const,
                               mapnik::new_geometry::multi_point const&, karma::domain>
    {
        using type = mapnik::new_geometry::multi_point const&;
        static type pre(mapnik::new_geometry::geometry const& geom)
        {
            return mapnik::util::get<mapnik::new_geometry::multi_point>(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::new_geometry::geometry const,
                               mapnik::new_geometry::multi_line_string const&, karma::domain>
    {
        using type = mapnik::new_geometry::multi_line_string const&;
        static type pre(mapnik::new_geometry::geometry const& geom)
        {
            return mapnik::util::get<mapnik::new_geometry::multi_line_string>(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::new_geometry::geometry const,
                               mapnik::new_geometry::multi_polygon const&, karma::domain>
    {
        using type = mapnik::new_geometry::multi_polygon const&;
        static type pre(mapnik::new_geometry::geometry const& geom)
        {
            return mapnik::util::get<mapnik::new_geometry::multi_polygon>(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::new_geometry::geometry const,
                               mapnik::new_geometry::geometry_collection const&, karma::domain>
    {
        using type = mapnik::new_geometry::geometry_collection const&;
        static type pre(mapnik::new_geometry::geometry const& geom)
        {
            return mapnik::util::get<mapnik::new_geometry::geometry_collection>(geom);
        }
    };

}}}

#endif // MAPNIK_UTIL_SPIRIT_TRANSFORM_ATTRIBUTE_HPP
