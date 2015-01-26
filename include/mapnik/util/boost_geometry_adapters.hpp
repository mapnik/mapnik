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

#ifndef BOOST_GEOMETRY_ADAPTERS_HPP
#define BOOST_GEOMETRY_ADAPTERS_HPP

// boost
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/register/point.hpp>

BOOST_GEOMETRY_REGISTER_POINT_2D(mapnik::coord2d, double, cs::cartesian, x, y)

// register mapnik::box2d<double>
namespace boost { namespace geometry { namespace traits
{

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

}}}


#endif //BOOST_GEOMETRY_ADAPTERS_HPP
