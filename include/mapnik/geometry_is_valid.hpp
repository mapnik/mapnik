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

enum y_axis_orientation_enum : std::uint8_t
{
    Y_AXIS_NORTH_POSITIVE,
    Y_AXIS_NORTH_NEGATIVE
};

template <typename T>
inline bool is_valid_rings(polygon<T> const& poly,
                           std::string &message,
                           y_axis_orientation_enum const& y_axis_orientation = Y_AXIS_NORTH_POSITIVE )
{

    // TODO: update to Boost 1.58, it supports getting the reason the ring is invalid
    //       it would be nice to use "b::g::is_valid" here, but it does not support reverse
    //       winding for inner rings, nor does it work when the Y-axis is inverted
    if (!boost::geometry::is_simple(poly.exterior_ring))
    {
        message = "Exterior ring is not simple";
        return false;
    }
    if (boost::geometry::intersects(poly.exterior_ring))
    {
        message = "Exterior ring self-intersects";
        return false;
    }
    int n=0;
    for (auto const& ring : poly.interior_rings)
    {
        // Can't use is_valid here, it retrns false on opposite windings for inner rings
        if (!boost::geometry::is_simple(ring))
        {
            message = "Interior ring " + std::to_string(n) + " is not simple";
            return false;
        }
        if (boost::geometry::intersects(ring))
        {
            message = "Interior ring " + std::to_string(n) + " self-intersects";
            return false;
        }
        ++n;
    }

    // Now, verify the winding directions of everything, according to OGC where "view from the top"
    // means exterior ring should be CCW, and interior rings should be CW.
    // If the y-axis orientation is north-positive, a CCW exterior ring should have a positive area,
    // and the CW interior rings should have positive areas.
    // If the y-axis orientation is north-negative, this logic is reversed as the area calculation
    // will return the inverse.
    if (y_axis_orientation == Y_AXIS_NORTH_POSITIVE && boost::geometry::area(poly.exterior_ring) < 0)
    {
        message = "Invalid winding for exterior ring under positive Y axis";
        return false;
    }
    else if (y_axis_orientation == Y_AXIS_NORTH_NEGATIVE && boost::geometry::area(poly.exterior_ring) > 0)
    {
        message = "Invalid winding for exterior ring under negative Y axis";
        return false;
    }

    n=0;
    for (auto const& ring : poly.interior_rings)
    {
        if (y_axis_orientation == Y_AXIS_NORTH_POSITIVE && boost::geometry::area(ring) > 0)
        {
            message = "Invalid winding for interior ring " + std::to_string(n) + " under positive Y axis";
            return false;
        }
        else if (y_axis_orientation == Y_AXIS_NORTH_NEGATIVE && boost::geometry::area(ring) < 0)
        {
            message = "invalid winding for interior ring " + std::to_string(n) + " under negative Y axis";
            return false;
        }
        ++n;
    }

    // Now, make sure all the interior rings are inside the exterior ring
    // OGC 6.1.11.1
    n=0;
    for (auto const& ring : poly.interior_rings) {
        if (!boost::geometry::within(ring, poly.exterior_ring))
        {
            message = "Interior ring " + std::to_string(n) + " not within exterior ring";
            return false;
        }
        ++n;
    }

    // Finally, check that none of the interior rings overlap each other.
    // OGC 6.1.11.1
    const size_t interior_ring_count = poly.interior_rings.size();
    for (size_t i=0; i < interior_ring_count; ++i)
    {
        auto const& A = poly.interior_rings.at(i);
        for (size_t j=i+1; j < interior_ring_count; ++j)
        {
            if (boost::geometry::intersects(A, poly.interior_rings.at(j))) {
                message = "Interior ring " + std::to_string(i) + " intersects interior ring " + std::to_string(j);
                return false;
            }
        }
    }

    message = "";
    return true;
}

template <typename T>
inline bool is_valid_rings(polygon<T> const& poly,
                           y_axis_orientation_enum const& y_axis_orientation = Y_AXIS_NORTH_POSITIVE )
{
    std::string dummy;
    return is_valid_rings(poly, dummy, y_axis_orientation);
}

template <typename T>
inline bool is_valid_rings(multi_polygon<T> const& multipoly,
                           std::string &message,
                           y_axis_orientation_enum const& y_axis_orientation = Y_AXIS_NORTH_POSITIVE )
{
    for(polygon<T> poly : multipoly)
    {
        if (!is_valid_rings(poly, message, y_axis_orientation)) return false;
    }
    return true;
}


template <typename T>
inline bool is_valid_rings(multi_polygon<T> const& multipoly,
                           y_axis_orientation_enum const& y_axis_orientation = Y_AXIS_NORTH_POSITIVE )
{
    std::string dummy;
    return is_valid_rings(multipoly, y_axis_orientation);
}



}}

#endif // BOOST_VERSION >= 1.56
#endif // MAPNIK_GEOMETRY_IS_VALID_HPP
