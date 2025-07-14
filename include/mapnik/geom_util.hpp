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

#ifndef MAPNIK_GEOM_UTIL_HPP
#define MAPNIK_GEOM_UTIL_HPP

// mapnik
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/coord.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/geometry/geometry_types.hpp>
#include <mapnik/geometry/point.hpp>

// stl
#include <cmath>
#include <vector>
#include <algorithm>

// boost
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/optional.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {
template<typename T>
bool clip_test(T p, T q, double& tmin, double& tmax)
{
    double r = 0;
    bool result = true;
    if (p < 0.0)
    {
        r = q / p;
        if (r > tmax)
            result = false;
        else if (r > tmin)
            tmin = r;
    }
    else if (p > 0.0)
    {
        r = q / p;
        if (r < tmin)
            result = false;
        else if (r < tmax)
            tmax = r;
    }
    else if (q < 0.0)
        result = false;
    return result;
}

template<typename T, typename Image>
bool clip_line(T& x0, T& y0, T& x1, T& y1, box2d<T> const& box)
{
    double tmin = 0.0;
    double tmax = 1.0;
    double dx = x1 - x0;
    if (clip_test<double>(-dx, x0, tmin, tmax))
    {
        if (clip_test<double>(dx, box.width() - x0, tmin, tmax))
        {
            double dy = y1 - y0;
            if (clip_test<double>(-dy, y0, tmin, tmax))
            {
                if (clip_test<double>(dy, box.height() - y0, tmin, tmax))
                {
                    if (tmax < 1.0)
                    {
                        x1 = static_cast<T>(x0 + tmax * dx);
                        y1 = static_cast<T>(y0 + tmax * dy);
                    }
                    if (tmin > 0.0)
                    {
                        x0 += static_cast<T>(tmin * dx);
                        y0 += static_cast<T>(tmin * dy);
                    }
                    return true;
                }
            }
        }
    }
    return false;
}

template<typename Iter>
inline bool point_inside_path(double x, double y, Iter start, Iter end)
{
    bool inside = false;
    double x0 = std::get<0>(*start);
    double y0 = std::get<1>(*start);

    double x1 = 0;
    double y1 = 0;
    while (++start != end)
    {
        if (std::get<2>(*start) == SEG_MOVETO)
        {
            x0 = std::get<0>(*start);
            y0 = std::get<1>(*start);
            continue;
        }
        x1 = std::get<0>(*start);
        y1 = std::get<1>(*start);

        if ((((y1 <= y) && (y < y0)) || ((y0 <= y) && (y < y1))) && (x < (x0 - x1) * (y - y1) / (y0 - y1) + x1))
            inside = !inside;
        x0 = x1;
        y0 = y1;
    }
    return inside;
}

inline bool point_in_circle(double x, double y, double cx, double cy, double r)
{
    double dx = x - cx;
    double dy = y - cy;
    double d2 = dx * dx + dy * dy;
    return (d2 <= r * r);
}

template<typename T>
inline T sqr(T x)
{
    return x * x;
}

inline double distance2(double x0, double y0, double x1, double y1)
{
    double dx = x1 - x0;
    double dy = y1 - y0;
    return sqr(dx) + sqr(dy);
}

inline double distance(double x0, double y0, double x1, double y1)
{
    return std::sqrt(distance2(x0, y0, x1, y1));
}

inline double point_to_segment_distance(double x, double y, double ax, double ay, double bx, double by)
{
    double len2 = distance2(ax, ay, bx, by);

    if (len2 < 1e-14)
    {
        return distance(x, y, ax, ay);
    }

    double r = ((x - ax) * (bx - ax) + (y - ay) * (by - ay)) / len2;
    if (r < 0)
    {
        return distance(x, y, ax, ay);
    }
    else if (r > 1)
    {
        return distance(x, y, bx, by);
    }
    double s = ((ay - y) * (bx - ax) - (ax - x) * (by - ay)) / len2;
    return std::fabs(s) * std::sqrt(len2);
}

template<typename Iter>
inline bool point_on_path(double x, double y, Iter start, Iter end, double tol)
{
    double x0 = std::get<0>(*start);
    double y0 = std::get<1>(*start);
    double x1 = 0;
    double y1 = 0;
    while (++start != end)
    {
        if (std::get<2>(*start) == SEG_MOVETO)
        {
            x0 = std::get<0>(*start);
            y0 = std::get<1>(*start);
            continue;
        }
        x1 = std::get<0>(*start);
        y1 = std::get<1>(*start);

        double distance = point_to_segment_distance(x, y, x0, y0, x1, y1);
        if (distance < tol)
            return true;
        x0 = x1;
        y0 = y1;
    }
    return false;
}

// filters
template<typename T>
struct bounding_box_filter
{
    using value_type = T;
    box2d<value_type> box_;
    explicit bounding_box_filter(box2d<value_type> const& box)
        : box_(box)
    {}

    bool pass(box2d<value_type> const& extent) const { return extent.intersects(box_); }
};

using filter_in_box = bounding_box_filter<double>;

template<typename T>
struct at_point_filter
{
    using value_type = T;
    box2d<value_type> box_;
    explicit at_point_filter(coord<value_type, 2> const& pt, double tol = 0)
        : box_(pt, pt)
    {
        box_.pad(tol);
    }

    bool pass(box2d<value_type> const& extent) const { return extent.intersects(box_); }
};

using filter_at_point = at_point_filter<double>;

////////////////////////////////////////////////////////////////////////////
template<typename PathType>
double path_length(PathType& path)
{
    double x0 = 0;
    double y0 = 0;
    double x1 = 0;
    double y1 = 0;
    path.rewind(0);
    unsigned command = path.vertex(&x0, &y0);
    if (command == SEG_END)
        return 0;
    double length = 0;
    while (SEG_END != (command = path.vertex(&x1, &y1)))
    {
        if (command == SEG_CLOSE)
            continue;
        length += distance(x0, y0, x1, y1);
        x0 = x1;
        y0 = y1;
    }
    return length;
}

template<typename PathType>
bool hit_test_first(PathType& path, double x, double y)
{
    bool inside = false;
    double x0 = 0;
    double y0 = 0;
    double x1 = 0;
    double y1 = 0;
    path.rewind(0);
    unsigned command = path.vertex(&x0, &y0);
    if (command == SEG_END)
    {
        return false;
    }
    while (SEG_END != (command = path.vertex(&x1, &y1)))
    {
        if (command == SEG_CLOSE)
        {
            break;
        }
        if (command == SEG_MOVETO)
        {
            x0 = x1;
            y0 = y1;
            continue;
        }

        if ((((y1 <= y) && (y < y0)) || ((y0 <= y) && (y < y1))) && (x < (x0 - x1) * (y - y1) / (y0 - y1) + x1))
            inside = !inside;

        x0 = x1;
        y0 = y1;
    }
    return inside;
}

namespace label {

template<typename PathType>
bool middle_point(PathType& path, double& x, double& y, boost::optional<double&> angle = boost::none)
{
    double x0 = 0;
    double y0 = 0;
    double x1 = 0;
    double y1 = 0;
    double mid_length = 0.5 * path_length(path);
    path.rewind(0);
    unsigned command = path.vertex(&x0, &y0);
    if (command == SEG_END)
        return false;
    if (std::abs(mid_length) < std::numeric_limits<double>::epsilon())
    {
        x = x0;
        y = y0;
        return true;
    }
    double dist = 0.0;
    while (SEG_END != (command = path.vertex(&x1, &y1)))
    {
        if (command == SEG_CLOSE)
            continue;
        double seg_length = distance(x0, y0, x1, y1);

        if (dist + seg_length >= mid_length)
        {
            double r = (mid_length - dist) / seg_length;
            x = x0 + (x1 - x0) * r;
            y = y0 + (y1 - y0) * r;
            if (angle)
            {
                *angle = atan2(y1 - y0, x1 - x0);
            }
            break;
        }
        dist += seg_length;
        x0 = x1;
        y0 = y1;
    }
    return true;
}

template<typename PathType>
bool centroid(PathType& path, double& x, double& y)
{
    geometry::point<double> p0, p1, move_to, start;

    path.rewind(0);
    unsigned command = path.vertex(&p0.x, &p0.y);
    if (command == SEG_END)
        return false;

    start = move_to = p0;

    double atmp = 0.0;
    double xtmp = 0.0;
    double ytmp = 0.0;
    unsigned count = 1;
    while (SEG_END != (command = path.vertex(&p1.x, &p1.y)))
    {
        switch (command)
        {
            case SEG_MOVETO:
                move_to = p1;
                break;
            case SEG_CLOSE:
                p1 = move_to;
            case SEG_LINETO:
                double dx0 = p0.x - start.x;
                double dy0 = p0.y - start.y;
                double dx1 = p1.x - start.x;
                double dy1 = p1.y - start.y;

                double ai = dx0 * dy1 - dx1 * dy0;
                atmp += ai;
                xtmp += (dx1 + dx0) * ai;
                ytmp += (dy1 + dy0) * ai;
                break;
        }
        p0 = p1;
        ++count;
    }

    if (count <= 2)
    {
        x = (start.x + p0.x) * 0.5;
        y = (start.y + p0.y) * 0.5;
        return true;
    }

    if (atmp != 0)
    {
        x = (xtmp / (3 * atmp)) + start.x;
        y = (ytmp / (3 * atmp)) + start.y;
    }
    else
    {
        x = p0.x;
        y = p0.y;
    }
    return true;
}

// Compute centroid over a set of paths
#if 0
template <typename Iter>
bool centroid_geoms(Iter start, Iter end, double & x, double & y)
{
  double x0 = 0.0;
  double y0 = 0.0;
  double x1 = 0.0;
  double y1 = 0.0;
  double start_x = x0;
  double start_y = y0;

  double atmp = 0.0;
  double xtmp = 0.0;
  double ytmp = 0.0;
  unsigned count = 0;

  while (start != end)
  {
    typename Iter::value_type const& geom = *start++;
    vertex_adapter path(geom);
    path.rewind(0);
    unsigned command = path.vertex(&x0, &y0);
    if (command == SEG_END) continue;

    if ( ! count++ ) {
      start_x = x0;
      start_y = y0;
    }

    while (SEG_END != (command = path.vertex(&x1, &y1)))
    {
        if (command == SEG_CLOSE) continue;
        double dx0 = x0 - start_x;
        double dy0 = y0 - start_y;
        double dx1 = x1 - start_x;
        double dy1 = y1 - start_y;
        double ai = dx0 * dy1 - dx1 * dy0;
        atmp += ai;
        xtmp += (dx1 + dx0) * ai;
        ytmp += (dy1 + dy0) * ai;
        x0 = x1;
        y0 = y1;
        ++count;
    }

  }

  if (count == 0) return false;

  if (count <= 2) {
      x = (start_x + x0) * 0.5;
      y = (start_y + y0) * 0.5;
      return true;
  }

  if (atmp != 0)
  {
      x = (xtmp/(3*atmp)) + start_x;
      y = (ytmp/(3*atmp)) + start_y;
  }
  else
  {
      x = x0;
      y = y0;
  }

  return true;
}

#endif

template<typename PathType>
bool hit_test(PathType& path, double x, double y, double tol)
{
    bool inside = false;
    double x0 = 0;
    double y0 = 0;
    double x1 = 0;
    double y1 = 0;
    double start_x = 0;
    double start_y = 0;
    path.rewind(0);
    unsigned command = path.vertex(&x0, &y0);
    if (command == SEG_END)
    {
        return false;
    }
    unsigned count = 0;
    mapnik::geometry::geometry_types geom_type = static_cast<mapnik::geometry::geometry_types>(path.type());
    while (SEG_END != (command = path.vertex(&x1, &y1)))
    {
        ++count;
        if (command == SEG_MOVETO)
        {
            x0 = x1;
            y0 = y1;
            start_x = x0;
            start_y = y0;
            continue;
        }
        else if (command == SEG_CLOSE)
        {
            x1 = start_x;
            y1 = start_y;
        }
        switch (geom_type)
        {
            case mapnik::geometry::geometry_types::Polygon: {
                if ((((y1 <= y) && (y < y0)) || ((y0 <= y) && (y < y1))) && (x < (x0 - x1) * (y - y1) / (y0 - y1) + x1))
                    inside = !inside;
                break;
            }
            case mapnik::geometry::geometry_types::LineString: {
                double distance = point_to_segment_distance(x, y, x0, y0, x1, y1);
                if (distance < tol)
                    return true;
                break;
            }
            default:
                break;
        }
        x0 = x1;
        y0 = y1;
    }

    // TODO - handle multi-point?
    if (count == 0) // one vertex
    {
        return distance(x, y, x0, y0) <= tol;
    }
    return inside;
}

} // namespace label
} // namespace mapnik

#endif // MAPNIK_GEOM_UTIL_HPP
