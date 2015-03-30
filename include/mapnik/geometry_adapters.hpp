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

#ifndef MAPNIK_GEOMETRY_ADAPTERS_HPP
#define MAPNIK_GEOMETRY_ADAPTERS_HPP

// undef B0 to workaround https://svn.boost.org/trac/boost/ticket/10467
#undef B0
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/linestring.hpp>
#include <boost/geometry/geometries/register/ring.hpp>
#include <boost/range.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/geometry/core/mutable_range.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

// register point
BOOST_GEOMETRY_REGISTER_POINT_2D (mapnik::geometry::point, double, cs::cartesian, x, y)
// ring
BOOST_GEOMETRY_REGISTER_RING(mapnik::geometry::linear_ring)

namespace boost {

template <>
struct range_iterator<mapnik::geometry::line_string>
{
    using type = mapnik::geometry::line_string::iterator;
};

template <>
struct range_const_iterator<mapnik::geometry::line_string>
{
    using type = mapnik::geometry::line_string::const_iterator;
};

inline mapnik::geometry::line_string::iterator
range_begin(mapnik::geometry::line_string & line) {return line.begin();}

inline mapnik::geometry::line_string::iterator
range_end(mapnik::geometry::line_string & line) {return line.end();}

inline mapnik::geometry::line_string::const_iterator
range_begin(mapnik::geometry::line_string const& line) {return line.begin();}

inline mapnik::geometry::line_string::const_iterator
range_end(mapnik::geometry::line_string const& line) {return line.end();}


namespace geometry { namespace traits {

template<> struct tag<mapnik::geometry::bounding_box> { using type = box_tag; };

template<> struct point_type<mapnik::geometry::bounding_box> { using type = mapnik::geometry::point; };


template <>
struct indexed_access<mapnik::geometry::bounding_box, min_corner, 0>
{
    static inline double get(mapnik::geometry::bounding_box const& b) { return b.p0.x;}
    static inline void set(mapnik::geometry::bounding_box& b, double value) { b.p0.x = value; }
};

template <>
struct indexed_access<mapnik::geometry::bounding_box, min_corner, 1>
{
    static inline double get(mapnik::geometry::bounding_box const& b) { return b.p0.y;}
    static inline void set(mapnik::geometry::bounding_box& b, double value) { b.p0.y = value; }
};

template <>
struct indexed_access<mapnik::geometry::bounding_box, max_corner, 0>
{
    static inline double get(mapnik::geometry::bounding_box const& b) { return b.p1.x;}
    static inline void set(mapnik::geometry::bounding_box& b, double value) { b.p1.x = value; }
};

template <>
struct indexed_access<mapnik::geometry::bounding_box, max_corner, 1>
{
    static inline double get(mapnik::geometry::bounding_box const& b) { return b.p1.y;}
    static inline void set(mapnik::geometry::bounding_box& b, double value) { b.p1.y = value; }
};

template<>
struct tag<mapnik::geometry::line_string>
{
    using type = linestring_tag;
};

// mapnik::geometry::polygon
template<>
struct tag<mapnik::geometry::polygon>
{
    using type = polygon_tag;
};

template <>
struct point_order<mapnik::geometry::linear_ring>
{
    static const order_selector value = counterclockwise;
};

template<>
struct tag<mapnik::geometry::multi_point>
{
    using type = multi_point_tag;
};

template<>
struct tag<mapnik::geometry::multi_line_string>
{
    using type = multi_linestring_tag;
};

template<> struct tag<mapnik::geometry::multi_polygon>
{
    using type = multi_polygon_tag;
};

// ring
template<> struct ring_const_type<mapnik::geometry::polygon>
{
    using type =  mapnik::geometry::linear_ring const&;
};

template<> struct ring_mutable_type<mapnik::geometry::polygon>
{
    using type = mapnik::geometry::linear_ring&;
};

// interior
template<> struct interior_const_type<mapnik::geometry::polygon>
{
    using type = std::vector<mapnik::geometry::linear_ring> const&;
};

template<> struct interior_mutable_type<mapnik::geometry::polygon>
{
    using type = std::vector<mapnik::geometry::linear_ring>&;
};

// exterior
template<>
struct exterior_ring<mapnik::geometry::polygon>
{
    static mapnik::geometry::linear_ring& get(mapnik::geometry::polygon & p)
    {
        return p.exterior_ring;
    }

    static mapnik::geometry::linear_ring const& get(mapnik::geometry::polygon const& p)
    {
        return p.exterior_ring;
    }
};

template<>
struct interior_rings<mapnik::geometry::polygon>
{
    using holes_type = std::vector<mapnik::geometry::linear_ring>;
    static holes_type&  get(mapnik::geometry::polygon & p)
    {
        return p.interior_rings;
    }

    static holes_type const& get(mapnik::geometry::polygon const& p)
    {
        return p.interior_rings;
    }
};


}}}


#endif //MAPNIK_GEOMETRY_ADAPTERS_HPP
