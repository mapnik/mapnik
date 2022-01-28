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

#ifndef MAPNIK_GRADIENT_HPP
#define MAPNIK_GRADIENT_HPP

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include <agg_trans_affine.h>
MAPNIK_DISABLE_WARNING_POP

// mapnik
#include <mapnik/color.hpp>

// stl
#include <vector>

namespace mapnik {

using stop_pair = std::pair<double, mapnik::color>;
using stop_array = std::vector<stop_pair>;

enum gradient_e { NO_GRADIENT, LINEAR, RADIAL };

enum gradient_unit_e {
    USER_SPACE_ON_USE,
    USER_SPACE_ON_USE_BOUNDING_BOX, // used to indicate % age values were specified. This are % of the svg image extent.
    OBJECT_BOUNDING_BOX //  (i.e., the abstract coordinate system where (0,0) is at the top/left of the object bounding
                        //  box and (1,1) is at the bottom/right of the object bounding box)
};

class MAPNIK_DECL gradient
{
    // transform
    agg::trans_affine transform_;
    // control points for the gradient, x1/y1 is the start point, x2/y2 the stop point.
    double x1_;
    double y1_;
    double x2_;
    double y2_;
    // for radial gradients r specifies the radius of the stop circle centered on x2/y2
    double r_;

    stop_array stops_;
    // units for the coordinates
    gradient_unit_e units_;
    gradient_e gradient_type_;

  public:
    gradient();
    gradient(gradient const& other);
    gradient(gradient&& other);
    gradient& operator=(gradient rhs);
    bool operator==(gradient const& other) const;
    void set_gradient_type(gradient_e grad);
    gradient_e get_gradient_type() const;

    void set_transform(agg::trans_affine const& transform);
    agg::trans_affine const& get_transform() const;

    void set_units(gradient_unit_e units);
    gradient_unit_e get_units() const;

    void add_stop(double offset, color const& c);
    bool has_stop() const;

    stop_array const& get_stop_array() const;

    void set_control_points(double x1, double y1, double x2, double y2, double r = 0);
    void get_control_points(double& x1, double& y1, double& x2, double& y2, double& r) const;
    void get_control_points(double& x1, double& y1, double& x2, double& y2) const;

  private:
    void swap(gradient& other) noexcept;
};
} // namespace mapnik

#endif // MAPNIK_GRADIENT_HPP
