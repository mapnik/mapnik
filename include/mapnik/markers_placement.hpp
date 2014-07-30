/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_MARKERS_PLACEMENT_HPP
#define MAPNIK_MARKERS_PLACEMENT_HPP

#include <mapnik/markers_placements/line.hpp>
#include <mapnik/markers_placements/point.hpp>
#include <mapnik/markers_placements/interior.hpp>
#include <mapnik/symbolizer_enumerations.hpp>

#include <boost/variant.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/function.hpp>

namespace mapnik
{

template <typename Locator, typename Detector>
class markers_placement_finder : mapnik::noncopyable
{
public:
    using markers_placement = boost::variant<markers_point_placement<Locator, Detector>,
                                             markers_line_placement<Locator, Detector>,
                                             markers_interior_placement<Locator, Detector>>;

    class get_point_visitor : public boost::static_visitor<bool>
    {
    public:
        get_point_visitor(double &x, double &y, double &angle, bool ignore_placement)
            : x_(x), y_(y), angle_(angle), ignore_placement_(ignore_placement)
        {
        }

        template <typename T>
        bool operator()(T &placement) const
        {
            return placement.get_point(x_, y_, angle_, ignore_placement_);
        }

    private:
        double &x_, &y_, &angle_;
        bool ignore_placement_;
    };

    markers_placement_finder(marker_placement_e placement_type,
                             Locator &locator,
                             box2d<double> const& size,
                             agg::trans_affine const& tr,
                             Detector &detector,
                             double spacing,
                             double max_error,
                             bool allow_overlap)
        : placement_(create(placement_type, locator, size, tr, detector, spacing, max_error, allow_overlap))
    {
    }

    /** Get a point where the marker should be placed.
     * Each time this function is called a new point is returned.
     * \param x     Return value for x position
     * \param y     Return value for x position
     * \param angle Return value for rotation angle
     * \param ignore_placement Whether to add selected position to detector
     * \return True if a place is found, false if none is found.
     */
    bool get_point(double &x, double &y, double &angle, bool ignore_placement)
    {
        return boost::apply_visitor(get_point_visitor(x, y, angle, ignore_placement), placement_);
    }

private:
    /** Factory function for particular placement implementations.
     */
    static markers_placement create(marker_placement_e placement_type,
                             Locator &locator,
                             box2d<double> const& size,
                             agg::trans_affine const& tr,
                             Detector &detector,
                             double spacing,
                             double max_error,
                             bool allow_overlap)
    {
        static const std::map<marker_placement_e, boost::function<markers_placement(
            Locator &locator,
            box2d<double> const& size,
            agg::trans_affine const& tr,
            Detector &detector,
            double spacing,
            double max_error,
            bool allow_overlap)>> factories =
            {
                { MARKER_POINT_PLACEMENT, boost::value_factory<markers_point_placement<Locator, Detector>>() },
                { MARKER_INTERIOR_PLACEMENT, boost::value_factory<markers_interior_placement<Locator, Detector>>() },
                { MARKER_LINE_PLACEMENT, boost::value_factory<markers_line_placement<Locator, Detector>>() }
            };
        return factories.at(placement_type)(locator, size, tr, detector, spacing, max_error, allow_overlap);
    }

    markers_placement placement_;
};

}

#endif // MAPNIK_MARKERS_PLACEMENT_HPP
