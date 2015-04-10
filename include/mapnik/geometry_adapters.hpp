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
//
#include <mapnik/box2d.hpp>

// register point
BOOST_GEOMETRY_REGISTER_POINT_2D (mapnik::geometry::point<double>, double, cs::cartesian, x, y)
// ring
BOOST_GEOMETRY_REGISTER_RING(mapnik::geometry::linear_ring<double>)

// needed by box2d<double>
BOOST_GEOMETRY_REGISTER_POINT_2D(mapnik::coord2d, double, cs::cartesian, x, y)

namespace boost {

template <>
struct range_iterator<mapnik::geometry::line_string<double> >
{
    using type = mapnik::geometry::line_string<double>::iterator;
};

template <>
struct range_const_iterator<mapnik::geometry::line_string<double> >
{
    using type = mapnik::geometry::line_string<double>::const_iterator;
};

inline mapnik::geometry::line_string<double>::iterator
range_begin(mapnik::geometry::line_string<double> & line) {return line.begin();}

inline mapnik::geometry::line_string<double>::iterator
range_end(mapnik::geometry::line_string<double> & line) {return line.end();}

inline mapnik::geometry::line_string<double>::const_iterator
range_begin(mapnik::geometry::line_string<double> const& line) {return line.begin();}

inline mapnik::geometry::line_string<double>::const_iterator
range_end(mapnik::geometry::line_string<double> const& line) {return line.end();}


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
template<>
struct tag<mapnik::geometry::line_string<double> >
{
    using type = linestring_tag;
};

// mapnik::geometry::polygon
template<>
struct tag<mapnik::geometry::polygon<double> >
{
    using type = polygon_tag;
};

template <>
struct point_order<mapnik::geometry::linear_ring<double> >
{
    static const order_selector value = counterclockwise;
};

template<>
struct tag<mapnik::geometry::multi_point<double> >
{
    using type = multi_point_tag;
};

template<>
struct tag<mapnik::geometry::multi_line_string<double> >
{
    using type = multi_linestring_tag;
};

template<> struct tag<mapnik::geometry::multi_polygon<double> >
{
    using type = multi_polygon_tag;
};

// ring
template<> struct ring_const_type<mapnik::geometry::polygon<double> >
{
    using type =  mapnik::geometry::linear_ring<double> const&;
};

template<> struct ring_mutable_type<mapnik::geometry::polygon<double> >
{
    using type = mapnik::geometry::linear_ring<double>&;
};

// interior
template<> struct interior_const_type<mapnik::geometry::polygon<double> >
{
    using type = std::vector<mapnik::geometry::linear_ring<double> > const&;
};

template<> struct interior_mutable_type<mapnik::geometry::polygon<double> >
{
    using type = std::vector<mapnik::geometry::linear_ring<double> >&;
};

// exterior
template<>
struct exterior_ring<mapnik::geometry::polygon<double> >
{
    static mapnik::geometry::linear_ring<double> & get(mapnik::geometry::polygon<double> & p)
    {
        return p.exterior_ring;
    }

    static mapnik::geometry::linear_ring<double> const& get(mapnik::geometry::polygon<double> const& p)
    {
        return p.exterior_ring;
    }
};

template<>
struct interior_rings<mapnik::geometry::polygon<double> >
{
    using holes_type = std::vector<mapnik::geometry::linear_ring<double> >;
    static holes_type&  get(mapnik::geometry::polygon<double> & p)
    {
        return p.interior_rings;
    }

    static holes_type const& get(mapnik::geometry::polygon<double> const& p)
    {
        return p.interior_rings;
    }
};


}}}


#endif //MAPNIK_GEOMETRY_ADAPTERS_HPP
