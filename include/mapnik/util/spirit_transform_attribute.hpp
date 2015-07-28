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

#include <mapnik/geometry.hpp>
#include <mapnik/util/variant.hpp>

#include <vector>
#include <cstdint>

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include <boost/spirit/include/karma.hpp>
#pragma GCC diagnostic pop

namespace boost { namespace spirit { namespace traits {

    template <>
    struct transform_attribute<mapnik::geometry::geometry<double> const,
                               mapnik::geometry::point<double> const&, karma::domain>
    {
        using type = mapnik::geometry::point<double> const&;
        static type pre(mapnik::geometry::geometry<double> const& geom)
        {
            return mapnik::util::get<mapnik::geometry::point<double> >(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::geometry<double> const,
                               mapnik::geometry::line_string<double> const&, karma::domain>
    {
        using type = mapnik::geometry::line_string<double> const&;
        static type pre(mapnik::geometry::geometry<double> const& geom)
        {
            return mapnik::util::get<mapnik::geometry::line_string<double> >(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::geometry<double> const,
                               mapnik::geometry::polygon<double> const&, karma::domain>
    {
        using type = mapnik::geometry::polygon<double> const&;
        static type pre(mapnik::geometry::geometry<double> const& geom)
        {
            return mapnik::util::get<mapnik::geometry::polygon<double> >(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::polygon<double> const,
                               std::vector<mapnik::geometry::linear_ring<double> > const&, karma::domain>
    {
        using type = std::vector<mapnik::geometry::linear_ring<double> > const&;
        static type pre(mapnik::geometry::polygon<double> const& poly)
        {
            return poly.interior_rings;
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::geometry<double> const,
                               mapnik::geometry::multi_point<double> const&, karma::domain>
    {
        using type = mapnik::geometry::multi_point<double> const&;
        static type pre(mapnik::geometry::geometry<double> const& geom)
        {
            return mapnik::util::get<mapnik::geometry::multi_point<double> >(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::geometry<double> const,
                               mapnik::geometry::multi_line_string<double> const&, karma::domain>
    {
        using type = mapnik::geometry::multi_line_string<double> const&;
        static type pre(mapnik::geometry::geometry<double> const& geom)
        {
            return mapnik::util::get<mapnik::geometry::multi_line_string<double> >(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::geometry<double> const,
                               mapnik::geometry::multi_polygon<double> const&, karma::domain>
    {
        using type = mapnik::geometry::multi_polygon<double> const&;
        static type pre(mapnik::geometry::geometry<double> const& geom)
        {
            return mapnik::util::get<mapnik::geometry::multi_polygon<double> >(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::geometry<double> const,
                               mapnik::geometry::geometry_collection<double> const&, karma::domain>
    {
        using type = mapnik::geometry::geometry_collection<double> const&;
        static type pre(mapnik::geometry::geometry<double> const& geom)
        {
            return mapnik::util::get<mapnik::geometry::geometry_collection<double> >(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::geometry<std::int64_t> const,
                               mapnik::geometry::point<std::int64_t> const&, karma::domain>
    {
        using type = mapnik::geometry::point<std::int64_t> const&;
        static type pre(mapnik::geometry::geometry<std::int64_t> const& geom)
        {
            return mapnik::util::get<mapnik::geometry::point<std::int64_t> >(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::geometry<std::int64_t> const,
                               mapnik::geometry::line_string<std::int64_t> const&, karma::domain>
    {
        using type = mapnik::geometry::line_string<std::int64_t> const&;
        static type pre(mapnik::geometry::geometry<std::int64_t> const& geom)
        {
            return mapnik::util::get<mapnik::geometry::line_string<std::int64_t> >(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::geometry<std::int64_t> const,
                               mapnik::geometry::polygon<std::int64_t> const&, karma::domain>
    {
        using type = mapnik::geometry::polygon<std::int64_t> const&;
        static type pre(mapnik::geometry::geometry<std::int64_t> const& geom)
        {
            return mapnik::util::get<mapnik::geometry::polygon<std::int64_t> >(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::polygon<std::int64_t> const,
                               std::vector<mapnik::geometry::linear_ring<std::int64_t> > const&, karma::domain>
    {
        using type = std::vector<mapnik::geometry::linear_ring<std::int64_t> > const&;
        static type pre(mapnik::geometry::polygon<std::int64_t> const& poly)
        {
            return poly.interior_rings;
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::geometry<std::int64_t> const,
                               mapnik::geometry::multi_point<std::int64_t> const&, karma::domain>
    {
        using type = mapnik::geometry::multi_point<std::int64_t> const&;
        static type pre(mapnik::geometry::geometry<std::int64_t> const& geom)
        {
            return mapnik::util::get<mapnik::geometry::multi_point<std::int64_t> >(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::geometry<std::int64_t> const,
                               mapnik::geometry::multi_line_string<std::int64_t> const&, karma::domain>
    {
        using type = mapnik::geometry::multi_line_string<std::int64_t> const&;
        static type pre(mapnik::geometry::geometry<std::int64_t> const& geom)
        {
            return mapnik::util::get<mapnik::geometry::multi_line_string<std::int64_t> >(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::geometry<std::int64_t> const,
                               mapnik::geometry::multi_polygon<std::int64_t> const&, karma::domain>
    {
        using type = mapnik::geometry::multi_polygon<std::int64_t> const&;
        static type pre(mapnik::geometry::geometry<std::int64_t> const& geom)
        {
            return mapnik::util::get<mapnik::geometry::multi_polygon<std::int64_t> >(geom);
        }
    };

    template <>
    struct transform_attribute<mapnik::geometry::geometry<std::int64_t> const,
                               mapnik::geometry::geometry_collection<std::int64_t> const&, karma::domain>
    {
        using type = mapnik::geometry::geometry_collection<std::int64_t> const&;
        static type pre(mapnik::geometry::geometry<std::int64_t> const& geom)
        {
            return mapnik::util::get<mapnik::geometry::geometry_collection<std::int64_t> >(geom);
        }
    };

}}}

#endif // MAPNIK_UTIL_SPIRIT_TRANSFORM_ATTRIBUTE_HPP
