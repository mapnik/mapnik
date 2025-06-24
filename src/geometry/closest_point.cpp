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
#include <boost/version.hpp>

#if BOOST_VERSION >= 106200
#include <mapnik/geometry.hpp>
#include <mapnik/geometry/boost_adapters.hpp>
#include <mapnik/geometry/closest_point.hpp>
#include <boost/geometry/extensions/algorithms/closest_point.hpp>
#include <boost/geometry/algorithms/within.hpp>

namespace mapnik {
namespace geometry {

namespace detail {

template<typename T>
struct closest_point
{
    using coordinate_type = T;
    using info_type = boost::geometry::closest_point_result<mapnik::geometry::point<coordinate_type>>;

    closest_point(mapnik::geometry::point<coordinate_type> const& pt)
        : pt_(pt)
    {}

    result_type operator()(mapnik::geometry::geometry_empty const&) const { return result_type(); }

    result_type operator()(mapnik::geometry::point<coordinate_type> const& pt) const
    {
        info_type info;
        boost::geometry::closest_point(pt_, pt, info);
        return result_type{info.closest_point.x, info.closest_point.y, info.distance};
    }

    result_type operator()(mapnik::geometry::line_string<coordinate_type> const& line) const
    {
        info_type info;
        boost::geometry::closest_point(pt_, line, info);
        return result_type{info.closest_point.x, info.closest_point.y, info.distance};
    }

    result_type operator()(mapnik::geometry::polygon<coordinate_type> const& poly) const
    {
        info_type info;
        if (boost::geometry::within(pt_, poly))
        {
            return result_type{pt_.x, pt_.y, 0.0};
        }
        bool first = true;
        for (auto const& ring : poly)
        {
            info_type ring_info;
            boost::geometry::closest_point(pt_, ring, ring_info);
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
        return result_type{info.closest_point.x, info.closest_point.y, info.distance};
    }

    // Multi* + GeometryCollection
    result_type operator()(mapnik::geometry::geometry<coordinate_type> const& geom) const
    {
        return mapnik::util::apply_visitor(*this, geom);
    }

    template<typename MultiGeometry>
    result_type operator()(MultiGeometry const& multi_geom) const
    {
        result_type result;
        bool first = true;
        for (auto const& geom : multi_geom)
        {
            if (first)
            {
                first = false;
                result = std::move(operator()(geom));
            }
            else
            {
                auto sub_result = operator()(geom);
                if (sub_result.distance < result.distance)
                {
                    result = std::move(sub_result);
                }
            }
        }
        return result;
    }
    mapnik::geometry::point<coordinate_type> pt_;
};

} // namespace detail

template<typename T1, typename T2>
MAPNIK_DECL result_type closest_point(T1 const& geom, mapnik::geometry::point<T2> const& pt)
{
    return detail::closest_point<T2>(pt)(geom);
}

template MAPNIK_DECL result_type closest_point(geometry<double> const&, point<double> const&);

template MAPNIK_DECL result_type closest_point(point<double> const&, point<double> const&);

template MAPNIK_DECL result_type closest_point(line_string<double> const&, point<double> const&);

template MAPNIK_DECL result_type closest_point(polygon<double> const&, point<double> const&);

template MAPNIK_DECL result_type closest_point(multi_point<double> const&, point<double> const&);

template MAPNIK_DECL result_type closest_point(multi_line_string<double> const&, point<double> const&);

template MAPNIK_DECL result_type closest_point(multi_polygon<double> const&, point<double> const&);

template MAPNIK_DECL result_type closest_point(geometry_empty const&, point<double> const&);

template MAPNIK_DECL result_type closest_point(geometry_collection<double> const&, point<double> const&);

} // namespace geometry
} // namespace mapnik

#endif //
