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

#ifndef MAPNIK_JSON_CREATE_GEOMETRY_HPP
#define MAPNIK_JSON_CREATE_GEOMETRY_HPP

#include <mapnik/geometry.hpp>
#include <mapnik/geometry/correct.hpp>
#include <mapnik/json/positions_x3.hpp>

namespace mapnik {
namespace json {

namespace {
// geometries
template<typename Geometry>
struct create_point
{
    explicit create_point(Geometry& geom)
        : geom_(geom)
    {}

    void operator()(point const& pos) const
    {
        mapnik::geometry::point<double> point(pos.x, pos.y);
        geom_ = std::move(point);
    }

    template<typename T>
    void operator()(T const&) const
    {
        throw std::runtime_error("Failed to parse geojson geometry");
    }
    Geometry& geom_;
};

template<typename Geometry>
struct create_linestring
{
    explicit create_linestring(Geometry& geom)
        : geom_(geom)
    {}

    void operator()(ring const& points) const
    {
        mapnik::geometry::line_string<double> line;
        std::size_t size = points.size();
        line.reserve(size);
        for (auto&& pt : points)
        {
            line.emplace_back(std::move(pt));
        }
        geom_ = std::move(line);
    }

    template<typename T>
    void operator()(T const&) const
    {
        throw std::runtime_error("Failed to parse geojson geometry");
    }

    Geometry& geom_;
};

template<typename Geometry>
struct create_polygon
{
    explicit create_polygon(Geometry& geom)
        : geom_(geom)
    {}

    void operator()(rings const& rngs) const
    {
        mapnik::geometry::polygon<double> poly;
        std::size_t num_rings = rngs.size();
        poly.reserve(num_rings);
        for (auto const& r : rngs)
        {
            std::size_t size = r.size();
            mapnik::geometry::linear_ring<double> ring;
            ring.reserve(size);
            for (auto&& pt : r)
            {
                ring.emplace_back(std::move(pt));
            }
            poly.push_back(std::move(ring));
        }
        geom_ = std::move(poly);
        mapnik::geometry::correct(geom_);
    }

    template<typename T>
    void operator()(T const&) const
    {
        throw std::runtime_error("Failed to parse geojson geometry");
    }

    Geometry& geom_;
};

// multi-geometries
template<typename Geometry>
struct create_multipoint
{
    explicit create_multipoint(Geometry& geom)
        : geom_(geom)
    {}

    void operator()(ring const& points) const
    {
        mapnik::geometry::multi_point<double> multi_point;
        multi_point.reserve(points.size());
        for (auto&& pos : points)
        {
            multi_point.emplace_back(std::move(pos));
        }
        geom_ = std::move(multi_point);
    }

    template<typename T>
    void operator()(T const&) const
    {
        throw std::runtime_error("Failed to parse geojson geometry");
    }

    Geometry& geom_;
};

template<typename Geometry>
struct create_multilinestring
{
    explicit create_multilinestring(Geometry& geom)
        : geom_(geom)
    {}

    void operator()(rings const& rngs) const
    {
        mapnik::geometry::multi_line_string<double> multi_line;
        multi_line.reserve(rngs.size());

        for (auto const& ring : rngs)
        {
            mapnik::geometry::line_string<double> line;
            line.reserve(ring.size());
            for (auto&& pt : ring)
            {
                line.emplace_back(std::move(pt));
            }
            multi_line.emplace_back(std::move(line));
        }
        geom_ = std::move(multi_line);
    }

    template<typename T>
    void operator()(T const&) const
    {
        throw std::runtime_error("Failed to parse geojson geometry");
    }

    Geometry& geom_;
};

template<typename Geometry>
struct create_multipolygon
{
    explicit create_multipolygon(Geometry& geom)
        : geom_(geom)
    {}

    void operator()(rings_array const& rngs_arr) const
    {
        mapnik::geometry::multi_polygon<double> multi_poly;
        multi_poly.reserve(rngs_arr.size());
        for (auto const& rings : rngs_arr)
        {
            mapnik::geometry::polygon<double> poly;
            std::size_t num_rings = rings.size();
            poly.reserve(num_rings);
            for (std::size_t i = 0; i < num_rings; ++i)
            {
                std::size_t size = rings[i].size();
                mapnik::geometry::linear_ring<double> ring;
                ring.reserve(size);
                for (auto&& pt : rings[i])
                {
                    ring.emplace_back(std::move(pt));
                }

                poly.push_back(std::move(ring));
            }
            multi_poly.emplace_back(std::move(poly));
        }
        geom_ = std::move(multi_poly);
        mapnik::geometry::correct(geom_);
    }

    template<typename T>
    void operator()(T const&) const
    {
        throw std::runtime_error("Failed to parse geojson geometry");
    }

    Geometry& geom_;
};
} // namespace

template<typename Geometry>
void create_geometry(Geometry& geom, int type, mapnik::json::positions const& coords)
{
    switch (type)
    {
        case 1: // Point
            util::apply_visitor(create_point<Geometry>(geom), coords);
            break;
        case 2: // LineString
            util::apply_visitor(create_linestring<Geometry>(geom), coords);
            break;
        case 3: // Polygon
            util::apply_visitor(create_polygon<Geometry>(geom), coords);
            break;
        case 4: // MultiPoint
            util::apply_visitor(create_multipoint<Geometry>(geom), coords);
            break;
        case 5: // MultiLineString
            util::apply_visitor(create_multilinestring<Geometry>(geom), coords);
            break;
        case 6: // MultiPolygon
            util::apply_visitor(create_multipolygon<Geometry>(geom), coords);
            break;
        default:
            throw std::runtime_error("Failed to parse geojson geometry");
            break;
    }
}

} // namespace json
} // namespace mapnik

#endif // MAPNIK_JSON_CREATE_GEOMETRY_HPP
