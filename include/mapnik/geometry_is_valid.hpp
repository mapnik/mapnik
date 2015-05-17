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
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/crosses.hpp>

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

typedef boost::geometry::model::d2::point_xy<double> Point;
typedef boost::geometry::model::segment<Point> Segment;

template <typename T>
std::vector<Segment> make_segment_vector(linear_ring<T> ring)
{
    std::vector<Segment> result;

    if (ring.size() > 1) {
        for (int i = 0; i < ring.size()-1; ++i)
        {
            Point a( ring.at(i).x, ring.at(i).y );
            Point b( ring.at(i+1).x, ring.at(i+1).y );
            Segment s(a, b);
            result.push_back(std::move(s));
        }
    }

    return result;

}

template <typename T>
inline bool is_valid_rings(polygon<T> const& poly)
{
    // If there are no interior rings, then the polygon must be valid
    if (poly.interior_rings.size() == 0) return true;

    // If there is no exterior rings, and any interior rings have points
    // then this is not a valid polygon
    if (poly.exterior_ring.size() == 0) 
    {
        for (linear_ring<T> interior_ring : poly.interior_rings) 
        {
            if (interior_ring.size() > 0) 
            {
                return false;
            }
        }
    }

    // Validate that all the rings are actually rings (i.e. they don't intersect with themselves)
    // i.e. they're simple polygons.
    if (boost::geometry::intersects(poly.exterior_ring))
    {
        return false;
    }
    for (auto ring : poly.interior_rings) 
    {
        if (boost::geometry::intersects(ring)) 
        {
            return false;
        }
    }

    // Now, verify the winding directions of everything:  CCW for the exterior ring, and
    // CW for any interior rings.
    if (boost::geometry::area(poly.exterior_ring) < 0) 
    {
        return false;
    }
    for (auto ring : poly.interior_rings) 
    {
        if (boost::geometry::area(ring) > 0) 
        {
            return false;
        }
    }

    // Quick test to see if the interior rings have at least one point
    // inside the exterior ring.  This test, combined with the line crossing
    // test next will ensure that all rings are inside the exterior.
    // Also, check that rings start/end on the same point
    for (linear_ring<T> ring : poly.interior_rings) 
    {
        if (ring.size() > 0) 
        {
            if (!boost::geometry::within(ring.front(), poly.exterior_ring)) 
            {
                return false;
            }
            if (ring.size() > 1) 
            {
                if (ring.front().x != ring.back().x || ring.front().y != ring.back().y) 
                {
                    return false;
                }
            }
        }
    }

    // Then, make sure there are no line intersections between the exterior ring
    // and any of the interior rings
    // TODO: implement boost::geometry::crosses, or convert this to Shamos–Hoey

    auto exterior_segments = make_segment_vector(poly.exterior_ring);

    for (auto interior_ring : poly.interior_rings) 
    {
        auto interior_segments = make_segment_vector(interior_ring);
        for (auto a : exterior_segments) 
        {
            for (auto b : interior_segments)
            {
                if (boost::geometry::intersects(a, b)) 
                {
                    return false;
                }
            }
        }
    }

    // Finally, check that none of the interior rings overlap each other.
    for (int i=0; i < poly.interior_rings.size()-1; i++) 
    {
        for (int j=i+1; j < poly.interior_rings.size(); j++) 
        {
            if (boost::geometry::intersects(poly.interior_rings.at(i), poly.interior_rings.at(j))) {
                return false;
            }
        }
    }

    return true;
}

template <typename T>
inline bool is_valid_rings(multi_polygon<T> const& multipoly) {
    for(polygon<T> poly : multipoly) 
    {
        if (!is_valid_rings(poly)) return false;
    }
}

}}

#endif // BOOST_VERSION >= 1.56
#endif // MAPNIK_GEOMETRY_IS_VALID_HPP
