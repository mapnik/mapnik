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

#ifndef MAPNIK_BOOST_GEOMETRY_ADAPTERS_HPP
#define MAPNIK_BOOST_GEOMETRY_ADAPTERS_HPP

#include <mapnik/config.hpp>

// undef B0 to workaround https://svn.boost.org/trac/boost/ticket/10467
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#undef B0
#if BOOST_VERSION == 106400
#include <boost/qvm/mat_operations.hpp>
#endif
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/ring.hpp>
#include <boost/geometry/geometries/register/linestring.hpp>
MAPNIK_DISABLE_WARNING_POP
// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/geometry/box2d.hpp>
// std
#include <cstdint>

BOOST_GEOMETRY_REGISTER_POINT_2D(mapnik::geometry::point<double>, double, boost::geometry::cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_POINT_2D(mapnik::geometry::point<std::int64_t>,
                                 std::int64_t,
                                 boost::geometry::cs::cartesian,
                                 x,
                                 y)
BOOST_GEOMETRY_REGISTER_LINESTRING_TEMPLATED(mapnik::geometry::line_string)
BOOST_GEOMETRY_REGISTER_RING_TEMPLATED(mapnik::geometry::linear_ring)
// needed by box2d<T>
BOOST_GEOMETRY_REGISTER_POINT_2D(mapnik::coord2d, double, boost::geometry::cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_POINT_2D(mapnik::coord2f, float, boost::geometry::cs::cartesian, x, y)

namespace mapnik {

template<typename CoordinateType>
struct const_interior_rings
{
    using polygon_type = mapnik::geometry::polygon<CoordinateType> const;
    using const_iterator = typename polygon_type::const_iterator;
    using iterator = const_iterator; // needed by boost::range_iterator
    using value_type = typename polygon_type::value_type;

    const_interior_rings(polygon_type const& poly)
        : poly_(poly)
    {}

    const_iterator begin() const
    {
        auto itr = poly_.cbegin();
        std::advance(itr, 1);
        return itr;
    }

    const_iterator end() const { return poly_.cend(); }

    std::size_t size() const { return poly_.empty() ? 0 : poly_.size() - 1; }

    value_type const& back() const { return poly_.back(); }
    polygon_type const& poly_;
};

template<typename CoordinateType>
struct interior_rings
{
    using polygon_type = mapnik::geometry::polygon<CoordinateType>;
    using iterator = typename polygon_type::iterator;
    using const_iterator = typename polygon_type::const_iterator;
    using value_type = typename polygon_type::value_type;

    interior_rings(polygon_type& poly)
        : poly_(poly)
    {}

    iterator begin()
    {
        auto itr = poly_.begin();
        std::advance(itr, 1);
        return itr;
    }

    iterator end() { return poly_.end(); }
    const_iterator begin() const
    {
        auto itr = poly_.cbegin();
        std::advance(itr, 1);
        return itr;
    }

    const_iterator end() const { return poly_.cend(); }

    void clear() { poly_.resize(1); }

    void resize(std::size_t size) { poly_.resize(size + 1); }

    std::size_t size() const { return poly_.empty() ? 0 : poly_.size() - 1; }

    void push_back(value_type const& val) { poly_.push_back(val); }
    value_type& back() { return poly_.back(); }
    value_type const& back() const { return poly_.back(); }
    polygon_type& poly_;
};

} // namespace mapnik

namespace boost {
namespace geometry {
namespace traits {

template<typename CoordinateType>
struct tag<mapnik::box2d<CoordinateType>>
{
    using type = box_tag;
};

template<>
struct point_type<mapnik::box2d<double>>
{
    using type = mapnik::coord2d;
};
template<>
struct point_type<mapnik::box2d<float>>
{
    using type = mapnik::coord2f;
};

template<typename CoordinateType>
struct indexed_access<mapnik::box2d<CoordinateType>, min_corner, 0>
{
    using ct = CoordinateType;
    static inline ct get(mapnik::box2d<CoordinateType> const& b) { return b.minx(); }
    static inline void set(mapnik::box2d<CoordinateType>& b, ct const& value) { b.set_minx(value); }
};

template<typename CoordinateType>
struct indexed_access<mapnik::box2d<CoordinateType>, min_corner, 1>
{
    using ct = CoordinateType;
    static inline ct get(mapnik::box2d<CoordinateType> const& b) { return b.miny(); }
    static inline void set(mapnik::box2d<CoordinateType>& b, ct const& value) { b.set_miny(value); }
};

template<typename CoordinateType>
struct indexed_access<mapnik::box2d<CoordinateType>, max_corner, 0>
{
    using ct = CoordinateType;
    static inline ct get(mapnik::box2d<CoordinateType> const& b) { return b.maxx(); }
    static inline void set(mapnik::box2d<CoordinateType>& b, ct const& value) { b.set_maxx(value); }
};

template<typename CoordinateType>
struct indexed_access<mapnik::box2d<CoordinateType>, max_corner, 1>
{
    using ct = CoordinateType;
    static inline ct get(mapnik::box2d<CoordinateType> const& b) { return b.maxy(); }
    static inline void set(mapnik::box2d<CoordinateType>& b, ct const& value) { b.set_maxy(value); }
};

template<typename CoordinateType>
struct tag<mapnik::geometry::polygon<CoordinateType>>
{
    using type = polygon_tag;
};

template<typename CoordinateType>
struct point_order<mapnik::geometry::linear_ring<CoordinateType>>
{
    static order_selector const value = counterclockwise;
};

template<typename CoordinateType>
struct tag<mapnik::geometry::multi_point<CoordinateType>>
{
    using type = multi_point_tag;
};

template<typename CoordinateType>
struct tag<mapnik::geometry::multi_line_string<CoordinateType>>
{
    using type = multi_linestring_tag;
};

template<typename CoordinateType>
struct tag<mapnik::geometry::multi_polygon<CoordinateType>>
{
    using type = multi_polygon_tag;
};

// ring
template<typename CoordinateType>
struct ring_const_type<mapnik::geometry::polygon<CoordinateType>>
{
    using type = typename mapnik::geometry::linear_ring<CoordinateType> const&;
};

template<typename CoordinateType>
struct ring_mutable_type<mapnik::geometry::polygon<CoordinateType>>
{
    using type = typename mapnik::geometry::linear_ring<CoordinateType>&;
};

// interior
template<typename CoordinateType>
struct interior_const_type<mapnik::geometry::polygon<CoordinateType>>
{
    using type = typename mapnik::const_interior_rings<CoordinateType> const;
};

template<typename CoordinateType>
struct interior_mutable_type<mapnik::geometry::polygon<CoordinateType>>
{
    using type = typename mapnik::interior_rings<CoordinateType>;
};

template<typename CoordinateType>
struct exterior_ring<mapnik::geometry::polygon<CoordinateType>>
{
    using ring_const_type = typename ring_const_type<mapnik::geometry::polygon<CoordinateType>>::type;
    using ring_mutable_type = typename ring_mutable_type<mapnik::geometry::polygon<CoordinateType>>::type;
    static ring_mutable_type get(mapnik::geometry::polygon<CoordinateType>& p)
    {
        if (p.empty())
            p.resize(1);
        return p[0];
    }

    static ring_const_type get(mapnik::geometry::polygon<CoordinateType> const& p)
    {
        if (p.empty())
            throw std::runtime_error("Exterior ring must be initialized!");
        return p[0];
    }
};

template<typename CoordinateType>
struct interior_rings<mapnik::geometry::polygon<CoordinateType>>
{
    using interior_const_type = typename interior_const_type<mapnik::geometry::polygon<CoordinateType>>::type;
    using interior_mutable_type = typename interior_mutable_type<mapnik::geometry::polygon<CoordinateType>>::type;

    static interior_const_type get(mapnik::geometry::polygon<CoordinateType> const& p)
    {
        return mapnik::const_interior_rings<CoordinateType>(p);
    }

    static interior_mutable_type get(mapnik::geometry::polygon<CoordinateType>& p)
    {
        return mapnik::interior_rings<CoordinateType>(p);
    }
};

template<typename CoordinateType>
struct resize<mapnik::interior_rings<CoordinateType>>
{
    static inline void apply(mapnik::interior_rings<CoordinateType> interiors, std::size_t new_size)
    {
        interiors.resize(new_size);
    }
};

template<typename CoordinateType>
struct clear<mapnik::interior_rings<CoordinateType>>
{
    static inline void apply(mapnik::interior_rings<CoordinateType> interiors) { interiors.clear(); }
};

template<typename CoordinateType>
struct push_back<mapnik::interior_rings<CoordinateType>>
{
    template<typename Ring>
    static inline void apply(mapnik::interior_rings<CoordinateType> interiors, Ring const& ring)
    {
        interiors.push_back(ring);
    }
};

} // namespace traits
} // namespace geometry
} // namespace boost

#endif // MAPNIK_BOOST_GEOMETRY_ADAPTERS_HPP
