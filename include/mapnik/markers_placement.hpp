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

// mapnik
#include <mapnik/box2d.hpp>

// boost
#include <boost/utility.hpp>

namespace mapnik {

template <typename Locator, typename Detector>
class markers_placement : boost::noncopyable
{
public:
    /** Constructor for markers_placement object.
     * \param locator  Path along which markers are placed (type: vertex source)
     * \param size     Size of the marker
     * \param detector Collision detection
     * \param spacing  Distance between markers. If the value is negative it is
     *                 converted to a positive value with similar magnitude, but
     *                 choosen to optimize marker placement. 0 = no markers
     */
    markers_placement(Locator &locator, box2d<double> size, Detector &detector, double spacing, double max_error, bool allow_overlap);
    /** Start again at first marker.
     * \note Returns the same list of markers only works when they were NOT added
     *       to the detector.
     */
    void rewind();
    /** Get a point where the marker should be placed.
     * Each time this function is called a new point is returned.
     * \param x     Return value for x position
     * \param y     Return value for x position
     * \param angle Return value for rotation angle
     * \param add_to_detector Add selected position to detector
     * \return True if a place is found, false if none is found.
     */
    bool get_point(double *x, double *y, double *angle, bool add_to_detector = true);
private:
    Locator &locator_;
    box2d<double> size_;
    Detector &detector_;
    double spacing_;
    double max_error_;
    bool allow_overlap_;

    bool done_;
    double last_x, last_y;
    double next_x, next_y;
    /** If a marker could not be placed at the exact point where it should
     * go the next marker's distance will be a bit lower. */
    double error_;
    double spacing_left_;
    unsigned marker_nr_;

    /** Rotates the size_ box and translates the position. */
    box2d<double> perform_transform(double angle, double dx, double dy);
    /** Automatically chooses spacing. */
    double find_optimal_spacing(double s);
    /** Set spacing_left_, adjusts error_ and performs sanity checks. */
    void set_spacing_left(double sl, bool allow_negative=false);
};

}

#endif // MAPNIK_MARKERS_PLACEMENT_HPP
