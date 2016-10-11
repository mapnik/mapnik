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

#include <mapnik/config.hpp>

// undef B0 to workaround https://svn.boost.org/trac/boost/ticket/10467
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#undef B0
#include <boost/geometry/geometries/register/linestring.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/ring.hpp>
// NOTE: ideally we would not include all of boost/geometry here to save on compile time
// however we need to pull in <boost/geometry/multi/multi.hpp> for things to work
// and once we do that the compile time is == to just including boost/geometry.hpp
#include <boost/geometry.hpp>
#pragma GCC diagnostic pop

#include <mapnik/geometry.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/box2d.hpp>

#include <cstdint>

// register point
BOOST_GEOMETRY_REGISTER_POINT_2D (mapnik::geometry::point<double>, double, boost::geometry::cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_POINT_2D (mapnik::geometry::point<std::int64_t>, std::int64_t, boost::geometry::cs::cartesian, x, y)
// ring
BOOST_GEOMETRY_REGISTER_RING_TEMPLATED(mapnik::geometry::linear_ring)
// needed by box2d<double>
BOOST_GEOMETRY_REGISTER_POINT_2D(mapnik::coord2d, double, boost::geometry::cs::cartesian, x, y)

namespace boost {

template <typename CoordinateType>
struct range_iterator<mapnik::geometry::line_string<CoordinateType> >
{
    using type = typename mapnik::geometry::line_string<CoordinateType>::iterator;
};

template <typename CoordinateType>
struct range_const_iterator<mapnik::geometry::line_string<CoordinateType> >
{
    using type = typename mapnik::geometry::line_string<CoordinateType>::const_iterator;
};

template <typename CoordinateType>
inline typename mapnik::geometry::line_string<CoordinateType>::iterator
range_begin(mapnik::geometry::line_string<CoordinateType> & line) {return line.begin();}

template <typename CoordinateType>
inline typename mapnik::geometry::line_string<CoordinateType>::iterator
range_end(mapnik::geometry::line_string<CoordinateType> & line) {return line.end();}

template <typename CoordinateType>
inline typename mapnik::geometry::line_string<CoordinateType>::const_iterator
range_begin(mapnik::geometry::line_string<CoordinateType> const& line) {return line.begin();}

template <typename CoordinateType>
inline typename mapnik::geometry::line_string<CoordinateType>::const_iterator
range_end(mapnik::geometry::line_string<CoordinateType> const& line) {return line.end();}

namespace geometry { namespace traits {

// register mapnik::box2d<double>
template<> struct tag<mapnik::box2d<double> > { using type = box_tag; };
template<> struct point_type<mapnik::box2d<double> > { using type = mapnik::coord2d; };

template <>
struct indexed_access<mapnik::box2d<double>, min_corner, 0>
{
    using ct = coordinate_type<mapnik::coord2d>::type;
    static inline ct get(mapnik::box2d<double> const& b) { return b.minx();}
    static inline void set(mapnik::box2d<double> &b, ct const& value) { b.set_minx(value); }
};

template <>
struct indexed_access<mapnik::box2d<double>, min_corner, 1>
{
    using ct = coordinate_type<mapnik::coord2d>::type;
    static inline ct get(mapnik::box2d<double> const& b) { return b.miny();}
    static inline void set(mapnik::box2d<double> &b, ct const& value) { b.set_miny(value); }
};

template <>
struct indexed_access<mapnik::box2d<double>, max_corner, 0>
{
    using ct = coordinate_type<mapnik::coord2d>::type;
    static inline ct get(mapnik::box2d<double> const& b) { return b.maxx();}
    static inline void set(mapnik::box2d<double> &b, ct const& value) { b.set_maxx(value); }
};

template <>
struct indexed_access<mapnik::box2d<double>, max_corner, 1>
{
    using ct = coordinate_type<mapnik::coord2d>::type;
    static inline ct get(mapnik::box2d<double> const& b) { return b.maxy();}
    static inline void set(mapnik::box2d<double> &b , ct const& value) { b.set_maxy(value); }
};

// mapnik::geometry::line_string
template<typename CoordinateType>
struct tag<mapnik::geometry::line_string<CoordinateType> >
{
    using type = linestring_tag;
};

// mapnik::geometry::polygon
template<typename CoordinateType>
struct tag<mapnik::geometry::polygon<CoordinateType> >
{
    using type = polygon_tag;
};

template <typename CoordinateType>
struct point_order<mapnik::geometry::linear_ring<CoordinateType> >
{
    static const order_selector value = counterclockwise;
};

template<typename CoordinateType>
struct tag<mapnik::geometry::multi_point<CoordinateType> >
{
    using type = multi_point_tag;
};

template<typename CoordinateType>
struct tag<mapnik::geometry::multi_line_string<CoordinateType> >
{
    using type = multi_linestring_tag;
};

template<typename CoordinateType>
struct tag<mapnik::geometry::multi_polygon<CoordinateType> >
{
    using type = multi_polygon_tag;
};

// ring
template <typename CoordinateType>
struct ring_const_type<mapnik::geometry::polygon<CoordinateType> >
{
    using type = typename mapnik::geometry::linear_ring<CoordinateType> const&;
};

template <typename CoordinateType>
struct ring_mutable_type<mapnik::geometry::polygon<CoordinateType> >
{
    using type = typename mapnik::geometry::linear_ring<CoordinateType>&;
};

// interior
template <typename CoordinateType>
struct interior_const_type<mapnik::geometry::polygon<CoordinateType> >
{
    using type = typename mapnik::geometry::polygon<CoordinateType>::rings_container const&;
};

template <typename CoordinateType>
struct interior_mutable_type<mapnik::geometry::polygon<CoordinateType> >
{
    using type = typename mapnik::geometry::polygon<CoordinateType>::rings_container&;
};

// exterior
template <typename CoordinateType>
struct exterior_ring<mapnik::geometry::polygon<CoordinateType> >
{
    static mapnik::geometry::linear_ring<CoordinateType> & get(mapnik::geometry::polygon<CoordinateType> & p)
    {
        return p.exterior_ring;
    }

    static mapnik::geometry::linear_ring<CoordinateType> const& get(mapnik::geometry::polygon<CoordinateType> const& p)
    {
        return p.exterior_ring;
    }
};

template <typename CoordinateType>
struct interior_rings<mapnik::geometry::polygon<CoordinateType> >
{
    using holes_type = typename mapnik::geometry::polygon<CoordinateType>::rings_container;
    static holes_type&  get(mapnik::geometry::polygon<CoordinateType> & p)
    {
        return p.interior_rings;
    }

    static holes_type const& get(mapnik::geometry::polygon<CoordinateType> const& p)
    {
        return p.interior_rings;
    }
};

}}}


#endif //MAPNIK_GEOMETRY_ADAPTERS_HPP
