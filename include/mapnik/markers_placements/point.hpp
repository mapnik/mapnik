/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_MARKERS_PLACEMENTS_POINT_HPP
#define MAPNIK_MARKERS_PLACEMENTS_POINT_HPP

#include <mapnik/markers_placements/point.hpp>
#include <mapnik/geom_util.hpp>

#include "agg_basics.h"
#include "agg_trans_affine.h"

namespace mapnik {

template <typename Locator, typename Detector>
class markers_point_placement
{
public:
    markers_point_placement(
        Locator &locator,
        box2d<double> const& size,
        agg::trans_affine const& tr,
        Detector &detector,
        double spacing,
        double max_error,
        bool allow_overlap)
        : locator_(locator),
          size_(size),
          tr_(tr),
          detector_(detector),
          spacing_(spacing),
          max_error_(max_error),
          allow_overlap_(allow_overlap),
          marker_width_((size_ * tr_).width()),
          done_(false)
    {
        rewind();
    }

    /** Start again at first marker.
     * \note Returns the same list of markers only works when they were NOT added
     *       to the detector.
     */
    void rewind()
    {
        locator_.rewind(0);
        done_ = false;
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
        if (done_)
        {
            return false;
        }

        if (locator_.type() == mapnik::geometry_type::types::LineString)
        {
            if (!label::middle_point(locator_, x, y))
            {
                done_ = true;
                return false;
            }
        }
        else
        {
            if (!label::centroid(locator_, x, y))
            {
                done_ = true;
                return false;
            }
        }

        angle = 0;
        agg::trans_affine matrix = tr_;
        matrix.translate(x,y);
        box2d<double> box = size_ * matrix;

        if (!allow_overlap_ && !detector_.has_placement(box))
        {
            return false;
        }

        if (!ignore_placement)
        {
            detector_.insert(box);
        }

        done_ = true;
        return true;
    }

protected:
    Locator &locator_;
    box2d<double> size_;
    agg::trans_affine tr_;
    Detector &detector_;
    double spacing_;
    double max_error_;
    bool allow_overlap_;
    double marker_width_;
    bool done_;

    /** Rotates the size_ box and translates the position. */
    box2d<double> perform_transform(double angle, double dx, double dy)
    {
        double x1 = size_.minx();
        double x2 = size_.maxx();
        double y1 = size_.miny();
        double y2 = size_.maxy();
        agg::trans_affine tr = tr_ * agg::trans_affine_rotation(angle)
            .translate(dx, dy);
        double xA = x1, yA = y1,
               xB = x2, yB = y1,
               xC = x2, yC = y2,
               xD = x1, yD = y2;
        tr.transform(&xA, &yA);
        tr.transform(&xB, &yB);
        tr.transform(&xC, &yC);
        tr.transform(&xD, &yD);
        box2d<double> result(xA, yA, xC, yC);
        result.expand_to_include(xB, yB);
        result.expand_to_include(xD, yD);
        return result;
    }
};

}

#endif // MAPNIK_MARKERS_PLACEMENTS_POINT_HPP

