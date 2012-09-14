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

#ifndef MAPNIK_IMAGE_SCALING_HPP
#define MAPNIK_IMAGE_SCALING_HPP

// stl
#include <string>

#include <boost/optional.hpp>
#include <mapnik/config.hpp>

namespace mapnik
{

enum scaling_method_e
{
    SCALING_NEAR=0,
    SCALING_BILINEAR,
    SCALING_BICUBIC,
    SCALING_SPLINE16,
    SCALING_SPLINE36,
    SCALING_HANNING,
    SCALING_HAMMING,
    SCALING_HERMITE,
    SCALING_KAISER,
    SCALING_QUADRIC,
    SCALING_CATROM,
    SCALING_GAUSSIAN,
    SCALING_BESSEL,
    SCALING_MITCHELL,
    SCALING_SINC,
    SCALING_LANCZOS,
    SCALING_BLACKMAN,
    SCALING_BILINEAR8
};

boost::optional<scaling_method_e> scaling_method_from_string(std::string const& name);
boost::optional<std::string> scaling_method_to_string(scaling_method_e scaling_method);

template <typename Image>
void scale_image_agg(Image & target,
                      Image const& source,
                      scaling_method_e scaling_method,
                      double scale_factor,
                      double x_off_f=0,
                      double y_off_f=0,
                      double filter_radius=2,
                      double ratio=1);

template <typename Image>
void scale_image_bilinear_old(Image & target,
                              Image const& source,
                              double x_off_f=0,
                              double y_off_f=0);

template <typename Image>
void scale_image_bilinear8(Image & target,
                           Image const& source,
                           double x_off_f=0,
                           double y_off_f=0);

}
#endif // MAPNIK_IMAGE_SCALING_HPP
