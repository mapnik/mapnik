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

#ifndef MAPNIK_GEOMETRY_INTERIOR_HPP
#define MAPNIK_GEOMETRY_INTERIOR_HPP

// mapnik
#include <mapnik/geom_util.hpp>
#include <mapnik/geometry/boost_adapters.hpp>
#include <mapnik/geometry/envelope.hpp>

// boost
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>

// stl
#include <cmath>
#include <vector>
#include <algorithm>

namespace mapnik { namespace geometry {

namespace detail {

using point_type = point<double>;

struct bisector
{
    bisector(point_type const& center, double angle)
        : center(center),
          sin(std::sin(angle)),
          cos(std::cos(angle))
    {
    }

    inline bool intersects(point_type const& p1,
                           point_type const& p2) const
    {
        // Based on bisector equation in normal form:
        // y * cos - x * sin = 0
        double d1 = (center.x - p1.x) * sin + (p1.y - center.y) * cos;
        double d2 = (center.x - p2.x) * sin + (p2.y - center.y) * cos;
        return (d1 < 0 && d2 > 0) || (d1 > 0 && d2 < 0);
    }

    inline point_type intersection(point_type const& p1,
                                   point_type const& p2) const
    {
        // Two lines given by four points:
        // line 1: p1, p2
        // line 2: center, (center.x + cos, center.y + sin)
        double denom = (p2.y - p1.y) * cos - (p2.x - p1.x) * sin;
        // if (denom == 0) return { }; // A caller must ensure lines are not parallel
        double c1 = center.x * sin - center.y * cos;
        double c2 = p1.x * p2.y - p1.y * p2.x;
        return { (c1 * (p1.x - p2.x) + cos * c2) / denom,
                 (c1 * (p1.y - p2.y) + sin * c2) / denom };
    }

    inline double distance_to_center(point_type const& p) const
    {
        return (p.x - center.x) * cos + (p.y - center.y) * sin;
    }

    point_type center;
    double sin, cos;
};

struct intersection
{
    intersection(point_type const& p, double d)
        : position(p), distance(d)
    {
    }

    point_type position;
    double distance; // from center
};

struct intersector
{
    intersector(point_type const& center_, unsigned bisector_count)
        : center(center_),
          sector_angle(M_PI / bisector_count),
          angle_epsilon(std::numeric_limits<double>::epsilon()),
          bisectors(),
          intersections_per_bisector(bisector_count)
    {
        for (unsigned i = 0; i < bisector_count; i++)
        {
            double angle = i * M_PI / bisector_count;
            bisectors.emplace_back(boost::in_place(center, angle));
        }
    }

    template <typename Path>
    void apply(Path & path)
    {
        point<double> p0, p1, move_to;
        unsigned command = SEG_END;
        int sector_p0, sector_p1;

        path.rewind(0);

        while (SEG_END != (command = path.vertex(&p0.x, &p0.y)))
        {
            switch (command)
            {
                case SEG_MOVETO:
                    move_to = p0;
                    process_vertex(p0, sector_p0);
                    break;
                case SEG_CLOSE:
                    p0 = move_to;
                case SEG_LINETO:
                    process_vertex(p0, sector_p0);
                    // We are interested only in segments
                    // going from one sector to another.
                    if (sector_p0 != sector_p1)
                    {
                        process_segment(p0, p1);
                    }
                    break;
            }
            p1 = p0;
            sector_p1 = sector_p0;
        }
    }

    inline void process_vertex(point_type const& vertex, int & sector)
    {
        double angle = 2.0 * M_PI + std::atan2(vertex.y - center.y,
                                               vertex.x - center.x);
        sector = angle / sector_angle;
        // For simplicity, disable bisector crossing vertex.
        if (std::abs(sector * sector_angle - angle) < angle_epsilon)
        {
            std::size_t bisector_index = sector % bisectors.size();
            bisectors[bisector_index] = boost::none;
            intersections_per_bisector[bisector_index].clear();
        }
    }

    inline void process_segment(point_type const& p1,
                                point_type const& p2)
    {
        // Find intersections between given segment and bisectors.
        // This can be optimized a bit, we know which bisectors intersect
        // by sectors of endpoints.
        for (std::size_t bi = 0; bi < bisectors.size(); bi++)
        {
            boost::optional<bisector> const& bisec = bisectors[bi];
            if (bisec && bisec->intersects(p1, p2))
            {
                point_type ip = bisec->intersection(p1, p2);
                double distance = bisec->distance_to_center(ip);
                intersections_per_bisector[bi].emplace_back(ip, distance);
            }
        }
    }

    const point_type center;
    const double sector_angle;
    const double angle_epsilon;
    std::vector<boost::optional<bisector>> bisectors;
    std::vector<std::vector<intersection>> intersections_per_bisector;
};

struct placement
{
    placement(point_type const& point_,
              std::size_t bisector_index,
              double distance_intersection,
              double excenter_coef)
        : position(point_),
          bisector_index(bisector_index),
          fitness(distance_intersection * (1.0 - excenter_coef))
    {
    }

    bool operator<(placement const& rhs) const
    {
        return fitness > rhs.fitness;
    }

    point_type position;
    std::size_t bisector_index;
    double fitness;
};

template <typename Point>
bool in_radius(Point const& p1, Point const& p2, double radius)
{
    return (std::pow(p1.x - p2.x, 2) +
            std::pow(p1.y - p2.y, 2)) <= std::pow(radius, 2);
}

inline bool near_fitness(double fitness1, double fitness2)
{
    const double coef = 0.25;
    return fitness2 <= fitness1 &&
        std::abs(fitness1 - fitness2) <= (fitness1 * coef);
}

inline bool has_near_placement(placement const& p,
                        std::vector<placement> const& placements,
                        double near_radius)
{
    for (auto const& near_placement : placements)
    {
        if (in_radius(p.position, near_placement.position, near_radius)
            && near_fitness(p.fitness, near_placement.fitness))
        {
            return true;
        }
    }
    return false;
}

inline void generate_placements(
    intersector const& ir,
    multi_point<double> const& intersection_points,
    double max_distance,
    std::vector<placement> & placements,
    std::vector<std::vector<placement>> & placements_ber_bisector)
{
    for (unsigned ipb = 0; ipb < ir.intersections_per_bisector.size(); ipb++)
    {
        auto const& intersections = ir.intersections_per_bisector[ipb];
        for (unsigned i = 1; i < intersections.size(); i += 2)
        {
            intersection const& low = intersections[i - 1];
            intersection const& high = intersections[i];
            const double positions[] = { 0.3, 0.5, 0.7 };
            for (double pos : positions)
            {
                // Put placement in between of two intersection points.
                point_type position(
                    low.position.x + (high.position.x - low.position.x) * pos,
                    low.position.y + (high.position.y - low.position.y) * pos);
                double distance_center = std::abs(low.distance +
                    (high.distance - low.distance) * pos);
                double distance_intersection = boost::geometry::distance(
                    position, intersection_points);
                placements.emplace_back(position,
                                        ipb,
                                        distance_intersection,
                                        distance_center / max_distance);
                placements_ber_bisector[ipb].push_back(placements.back());
            }
        }
    }
}

inline bool find_best_placement(
    std::vector<placement> const& placements,
    std::vector<std::vector<placement>> const& placements_ber_bisector,
    double max_distance,
    double & x,
    double & y)
{
    // Max distance to neighbour placement.
    const double near_radius = max_distance / 4.0;
    const unsigned bisector_count = placements_ber_bisector.size();

    for (auto const & p : placements)
    {
        std::size_t prev_bisector_index = (bisector_count + p.bisector_index - 1) % bisector_count;
        std::size_t next_bisector_index = (p.bisector_index + 1) % bisector_count;

        bool has_prev_placement = has_near_placement(
            p,
            placements_ber_bisector[prev_bisector_index],
            near_radius);
        bool has_next_placement = has_near_placement(
            p,
            placements_ber_bisector[next_bisector_index],
            near_radius);

        if (has_prev_placement && has_next_placement)
        {
            x = p.position.x;
            y = p.position.y;
            return true;
        }
    }
    return false;
}

template <typename Path>
bool interior(Path & path, double & x, double & y, unsigned bisector_count)
{
    // Start with the centroid.
    if (!label::centroid(path, x, y))
    {
        return false;
    }

    if (bisector_count == 0)
    {
        return true;
    }

    const point_type center(x, y);

    // Find intersections between bisectors and input polygon.
    intersector ir(center, bisector_count);
    ir.apply(path);

    multi_point<double> intersection_points;
    // Sort intersection points by distance to the center for each bisector.
    // Collect all intersection points to one multi_point structure.
    for (auto & intersections : ir.intersections_per_bisector)
    {
        for (auto const& i : intersections)
        {
            intersection_points.push_back(i.position);
        }

        std::sort(intersections.begin(), intersections.end(),
            [](intersection const& i1, intersection const& i2) {
                return i1.distance < i2.distance;
            });
    }

    if (intersection_points.size() == 0)
    {
        return true;
    }

    const box2d<double> bbox = envelope(intersection_points);
    // Maximum distance from the center to an intersection.
    const double max_distance = std::max(bbox.width(), bbox.height());
    std::vector<placement> placements;
    std::vector<std::vector<placement>> placements_ber_bisector(bisector_count);

    // Generate all possible placements.
    generate_placements(ir, intersection_points, max_distance,
                        placements, placements_ber_bisector);

    // Sort placements by fitness.
    std::sort(placements.begin(), placements.end());

    // The best placement has not only top fitness, but also has
    // neighbour placements with similar fitness.
    // This simple approach filters out excesses caused by inaccuracy
    // of limited number of bisectors.
    if (find_best_placement(placements,
                            placements_ber_bisector,
                            max_distance,
                            x, y))
    {
        return true;
    }

    // If no best placement found, take the one with top fitness.
    x = placements.front().position.x;
    y = placements.front().position.y;

    return true;
}

} // namespace detail

template <typename Path>
bool interior(Path & path, double & x, double & y, unsigned bisector_count = 64)
{
    return detail::interior(path, x, y, bisector_count);
}

} }

#endif // MAPNIK_GEOMETRY_INTERIOR_HPP
