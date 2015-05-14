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

#ifndef MAPNIK_GEOMETRY_IS_VALID_HPP
#define MAPNIK_GEOMETRY_IS_VALID_HPP

#include <boost/version.hpp>

// only Boost >= 1.56 contains the is_valid function
#if BOOST_VERSION >= 105600

#include <mapnik/geometry.hpp>
#include <mapnik/geometry_adapters.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/algorithms/intersection.hpp>

namespace mapnik { namespace geometry {

namespace detail {

struct geometry_is_valid
{
    using result_type = bool;

    template <typename T>
    result_type operator() (geometry<T> const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    result_type operator() (geometry_empty const& ) const
    {
        return false;
    }

    template <typename T>
    result_type operator() (geometry_collection<T> const& collection) const
    {
        for (auto const& geom : collection)
        {
            if ( !(*this)(geom)) return false;
        }
        return true;
    }

    template <typename T>
    result_type operator() (point<T> const& pt) const
    {
        return boost::geometry::is_valid(pt);
    }

    template <typename T>
    result_type operator() (line_string<T> const& line) const
    {
        return boost::geometry::is_valid(line);
    }

    template <typename T>
    result_type operator() (polygon<T> const& poly) const
    {
        return boost::geometry::is_valid(poly);
    }

    template <typename T>
    result_type operator() (multi_point<T> const& multi_pt) const
    {
        return boost::geometry::is_valid(multi_pt);
    }

    template <typename T>
    result_type operator() (multi_line_string<T> const& multi_line) const
    {
        return boost::geometry::is_valid(multi_line);
    }

    template <typename T>
    result_type operator() (multi_polygon<T> const& multi_poly) const
    {
        return boost::geometry::is_valid(multi_poly);
    }
};

}

template <typename T>
inline bool is_valid(T const& geom)
{
    return detail::geometry_is_valid() (geom);
}

typedef boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian> Point;
typedef boost::geometry::model::segment<Point> Segment;

template <typename T>
std::vector<Segment> make_segment_vector(linear_ring<T> ring)
{

    std::vector<Segment> result;

    for (int i = 0; i < ring.size()-1; ++i)
    {
        Point a( ring.at(i).x, ring.at(i).y );
        Point b( ring.at(i+1).x, ring.at(i+1).y );
        Segment s(a, b);
        result.push_back(std::move(s));
    }
    Point a( ring.back().x, ring.back().y );
    Point b( ring.front().x, ring.front().y );
    Segment s(a, b);
    result.push_back(std::move(s));

    return result;

}

template <typename T>
inline bool is_valid_rings(polygon<T> const& poly)
{
    // First, test that at least one point from each interior ring is inside
    // the exterior ring.  We just grab the first.  If this is satisfied,
    // and the second test passes, then e
    for (linear_ring<T> interior_ring : poly.interior_rings) {
        if (!boost::geometry::within(interior_ring.front(), poly.exterior_ring)) {
            return false;
        }
    }

    std::vector<Segment> exterior_segments = make_segment_vector(poly.exterior_ring);
    // Then, make sure there are no line intersections between the exterior ring
    // and any of the interior rings
    for (linear_ring<T> interior_ring : poly.interior_rings) {
        std::vector<Segment> interior_segments = make_segment_vector(interior_ring);
        for (Segment interior_segment : interior_segments) {
            for (Segment exterior_segment : exterior_segments) {
                if (boost::geometry::intersects(interior_segment, exterior_segment)) {
                    return false;
                }
            }
        }
    }

    return true;

}

}}

#endif // BOOST_VERSION >= 1.56
#endif // MAPNIK_GEOMETRY_IS_VALID_HPP
