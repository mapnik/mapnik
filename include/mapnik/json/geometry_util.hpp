 /*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_JSON_GEOMETRY_UTIL_HPP
#define MAPNIK_JSON_GEOMETRY_UTIL_HPP

#include <mapnik/geometry.hpp>  // for geometry_type
#include <mapnik/vertex.hpp>  // for CommandType
#include <mapnik/make_unique.hpp>

namespace mapnik { namespace json {

// geometries
template <typename Path>
struct create_point : util::static_visitor<>
{
    explicit create_point(Path & path)
        : path_(path) {}

    void operator()(position const& pos) const
    {
        auto pt = std::make_unique<geometry_type>(geometry_type::types::Point);
        pt->move_to(std::get<0>(pos), std::get<1>(pos));
        path_.push_back(pt.release());
    }

    template <typename T>
    void operator()(T const&) const {} // no-op - shouldn't get here
    Path & path_;
};

template <typename Path>
struct create_linestring : util::static_visitor<>
{
    explicit create_linestring(Path & path)
        : path_(path) {}

    void operator()(positions const& ring) const
    {
        std::size_t size = ring.size();
        if (size > 1)
        {
            auto line = std::make_unique<geometry_type>(geometry_type::types::LineString);
            line->move_to(std::get<0>(ring.front()), std::get<1>(ring.front()));

            for (std::size_t index = 1; index < size; ++index)
            {
                line->line_to(std::get<0>(ring[index]), std::get<1>(ring[index]));
            }
            path_.push_back(line.release());
        }
    }

    template <typename T>
    void operator()(T const&) const {}  // no-op - shouldn't get here

    Path & path_;
};

template <typename Path>
struct create_polygon : util::static_visitor<>
{
    explicit create_polygon(Path & path)
        : path_(path) {}

    void operator()(std::vector<positions> const& rings) const
    {
        auto poly = std::make_unique<geometry_type>(geometry_type::types::Polygon);

        for (auto const& ring : rings)
        {
            std::size_t size = ring.size();
            if (size > 2) // at least 3 vertices to form a ring
            {
                poly->move_to(std::get<0>(ring.front()), std::get<1>(ring.front()));
                for (std::size_t index = 1; index < size; ++index)
                {
                    poly->line_to(std::get<0>(ring[index]), std::get<1>(ring[index]));
                }
                poly->close_path();
            }
        }
        path_.push_back(poly.release());
    }

    template <typename T>
    void operator()(T const&) const {}  // no-op - shouldn't get here

    Path & path_;
};

// multi-geometries

template <typename Path>
struct create_multipoint : util::static_visitor<>
{
    explicit create_multipoint(Path & path)
        : path_(path) {}

    void operator()(positions const& points) const
    {
        for (auto const& pos : points)
        {
            auto point = std::make_unique<geometry_type>(geometry_type::types::Point);
            point->move_to(std::get<0>(pos), std::get<1>(pos));
            path_.push_back(point.release());
        }
    }

    template <typename T>
    void operator()(T const&) const {}  // no-op - shouldn't get here

    Path & path_;
};

template <typename Path>
struct create_multilinestring : util::static_visitor<>
{
    explicit create_multilinestring(Path & path)
        : path_(path) {}

    void operator()(std::vector<positions> const& rings) const
    {
        for (auto const& ring : rings)
        {
            auto line = std::make_unique<geometry_type>(geometry_type::types::LineString);
            std::size_t size = ring.size();
            if (size > 1) // at least 2 vertices to form a linestring
            {
                line->move_to(std::get<0>(ring.front()), std::get<1>(ring.front()));
                for (std::size_t index = 1; index < size; ++index)
                {
                    line->line_to(std::get<0>(ring[index]), std::get<1>(ring[index]));
                }
            }
            path_.push_back(line.release());
        }
    }

    template <typename T>
    void operator()(T const&) const {}  // no-op - shouldn't get here

    Path & path_;
};

template <typename Path>
struct create_multipolygon : util::static_visitor<>
{
    explicit create_multipolygon(Path & path)
        : path_(path) {}

    void operator()(std::vector<std::vector<positions> > const& rings_array) const
    {
        for (auto const& rings : rings_array)
        {
            auto poly = std::make_unique<geometry_type>(geometry_type::types::Polygon);
            for (auto const& ring : rings)
            {
                std::size_t size = ring.size();
                if (size > 2) // at least 3 vertices to form a ring
                {
                    poly->move_to(std::get<0>(ring.front()), std::get<1>(ring.front()));
                    for (std::size_t index = 1; index < size; ++index)
                    {
                        poly->line_to(std::get<0>(ring[index]), std::get<1>(ring[index]));
                    }
                    poly->close_path();
                }
            }
            path_.push_back(poly.release());
        }
    }

    template <typename T>
    void operator()(T const&) const {}  // no-op - shouldn't get here

    Path & path_;
};

struct create_geometry_impl
{
    using result_type = void;
    template <typename T0>
    void operator() (T0 & path, int type, mapnik::json::coordinates const& coords) const
    {
        switch (type)
        {
        case 1 ://Point
            util::apply_visitor(create_point<T0>(path), coords);
            break;
        case 2 ://LineString
            util::apply_visitor(create_linestring<T0>(path), coords);
            break;
        case 3 ://Polygon
            util::apply_visitor(create_polygon<T0>(path), coords);
            break;
        case 4 ://MultiPoint
            util::apply_visitor(create_multipoint<T0>(path), coords);
            break;
        case 5 ://MultiLineString
            util::apply_visitor(create_multilinestring<T0>(path), coords);
            break;
        case 6 ://MultiPolygon
            util::apply_visitor(create_multipolygon<T0>(path), coords);
            break;
        default:
            break;
        }
    }
};

}}

#endif // MAPNIK_JSON_GEOMETRY_UTIL_HPP
