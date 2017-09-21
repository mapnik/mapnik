/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_GEOMETRY_CLOSEST_POINT_HPP
#define MAPNIK_GEOMETRY_CLOSEST_POINT_HPP

#include <boost/version.hpp>

#if BOOST_VERSION >= 106200

#include <mapnik/geometry.hpp>
#include <mapnik/geometry/boost_adapters.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/algorithms/comparable_distance.hpp>
#include <boost/geometry/extensions/algorithms/closest_point.hpp>

namespace mapnik { namespace geometry {

namespace detail {

template <typename T>
struct closest_point
{
    using coordinate_type = T;
    using result_type = boost::geometry::closest_point_result<mapnik::geometry::point<coordinate_type>>;

    closest_point(mapnik::geometry::point<coordinate_type> const& pt)
        : pt_(pt) {}

    result_type operator() (mapnik::geometry::geometry_empty const&) const
    {
        return result_type(); // FIXME: consider std::optional<result_type>
    }

    result_type operator() (mapnik::geometry::point<coordinate_type> const& pt) const
    {
        result_type info;
        boost::geometry::closest_point(pt_ ,pt, info);
        return info;
    }

    result_type operator() (mapnik::geometry::line_string<coordinate_type> const& line) const
    {
        result_type info;
        boost::geometry::closest_point(pt_ ,line, info);
        return info;
    }

    result_type operator() (mapnik::geometry::polygon<coordinate_type> const& poly) const
    {
        result_type info;
        if (boost::geometry::within(pt_, poly))
        {
            info.closest_point = pt_;
            info.distance = 0.0;
            return info;
        }
        bool first = true;
        for (auto const& ring : poly)
        {
            result_type ring_info;
            boost::geometry::closest_point(pt_ ,ring, ring_info);
            if (first)
            {
                first = false;
                info = std::move(ring_info);
            }
            else if (ring_info.distance < info.distance)
            {
                info = std::move(ring_info);
            }
        }
        return info;
    }



    // Multi* + GeometryCollection
    result_type operator() (mapnik::geometry::geometry<coordinate_type> const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    template <typename MultiGeometry>
    result_type operator() (MultiGeometry const& multi_geom) const
    {
        result_type info;
        bool first = true;
        for (auto const& geom : multi_geom)
        {
            if (first)
            {
                first = false;
                info = std::move(operator()(geom));
            }
            else
            {
                auto sub_info = operator()(geom);
                if (sub_info.distance < info.distance)
                {
                    info = std::move(sub_info);
                }
            }
        }
        return info;
    }

    mapnik::geometry::point<coordinate_type> pt_;
};

}

template <typename T1, typename T2>
inline typename detail::closest_point<T2>::result_type
closest_point(T1 const& geom, mapnik::geometry::point<T2> const& pt)
{
    return detail::closest_point<T2>(pt)(geom);
}

}}


#endif //
#endif // MAPNIK_GEOMETRY_CLOSEST_POINT_HPP
