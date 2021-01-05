/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_HIT_TEST_FILTER_HPP
#define MAPNIK_HIT_TEST_FILTER_HPP

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geom_util.hpp>

namespace mapnik {

namespace detail {

inline bool pip(double x0,
                double y0,
                double x1,
                double y1,
                double x,
                double y)
{
    return ((((y1 <= y) && (y < y0)) || ((y0 <= y) && (y < y1))) && (x < (x0 - x1) * (y - y1) / (y0 - y1) + x1));
}

struct hit_test_visitor
{
    hit_test_visitor(double x, double y, double tol)
     : x_(x),
       y_(y),
       tol_(tol) {}

    bool operator() (geometry::geometry_empty const& ) const
    {
        return false;
    }

    bool operator() (geometry::point<double> const& geom) const
    {
        return distance(geom.x, geom.y, x_, y_) <= tol_;
    }
    bool operator() (geometry::multi_point<double> const& geom) const
    {
        for (auto const& pt : geom)
        {
            if (distance(pt.x, pt.y, x_, y_) <= tol_) return true;
        }
        return false;
    }
    bool operator() (geometry::line_string<double> const& geom) const
    {
        std::size_t num_points = geom.size();
        if (num_points > 1)
        {
            for (std::size_t i = 1; i < num_points; ++i)
            {
                auto const& pt0 = geom[i-1];
                auto const& pt1 = geom[i];
                double distance = point_to_segment_distance(x_,y_,pt0.x,pt0.y,pt1.x,pt1.y);
                if (distance < tol_) return true;
            }
        }
        return false;
    }
    bool operator() (geometry::multi_line_string<double> const& geom) const
    {
        for (auto const& line: geom)
        {
            if (operator()(line)) return true;
        }
        return false;
    }
    bool operator() (geometry::polygon<double> const& poly) const
    {
        //auto const& exterior = geom.exterior_ring;
        //std::size_t num_points = exterior.size();
        //if (num_points < 4) return false;

        //for (std::size_t i = 1; i < num_points; ++i)
        //{
        //   auto const& pt0 = exterior[i-1];
        //   auto const& pt1 = exterior[i];
            // todo - account for tolerance
        //   if (pip(pt0.x,pt0.y,pt1.x,pt1.y,x_,y_))
        //   {
        //       inside = !inside;
        //   }
        //}
        //if (!inside) return false;

        //// FIXME !!!
        bool inside = false;
        bool exterior = true;
        for (auto const& ring :  poly)
        {
            std::size_t num_points = ring.size();
            if (num_points < 4)
            {
                if (exterior) return false;
                else continue;
            }

            for (std::size_t j = 1; j < num_points; ++j)
            {
                auto const& pt0 = ring[j - 1];
                auto const& pt1 = ring[j];
                if (pip(pt0.x, pt0.y, pt1.x, pt1.y, x_, y_))
                {
                    // TODO - account for tolerance
                    inside = !inside;
                }
            }
        }
        ////////////////////////////
        return inside;
    }
    bool operator() (geometry::multi_polygon<double> const& geom) const
    {
        for (auto const& poly: geom)
        {
            if (operator()(poly)) return true;
        }
        return false;
    }
    bool operator() (geometry::geometry_collection<double> const& collection) const
    {
        for (auto const& geom: collection)
        {
            if (mapnik::util::apply_visitor((*this),geom)) return true;
        }
        return false;
    }

    double x_;
    double y_;
    double tol_;
};

}

inline bool hit_test(mapnik::geometry::geometry<double> const& geom, double x, double y, double tol)
{
    return mapnik::util::apply_visitor(detail::hit_test_visitor(x,y,tol), geom);
}

class hit_test_filter
{
public:
    hit_test_filter(double x, double y, double tol)
        : x_(x),
          y_(y),
          tol_(tol) {}

    bool pass(feature_impl const& feature)
    {
        return hit_test(feature.get_geometry(),x_,y_,tol_);
    }

private:
    double x_;
    double y_;
    double tol_;
};
}

#endif // MAPNIK_HIT_TEST_FILTER_HPP
