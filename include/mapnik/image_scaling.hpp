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

#ifndef MAPNIK_IMAGE_SCALING_HPP
#define MAPNIK_IMAGE_SCALING_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/image.hpp>

// stl
#include <string>
#include <optional>

namespace mapnik {

enum scaling_method_e {
    SCALING_NEAR = 0,
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
    SCALING_BLACKMAN
};

MAPNIK_DECL std::optional<scaling_method_e> scaling_method_from_string(std::string const& name);
MAPNIK_DECL std::optional<std::string> scaling_method_to_string(scaling_method_e scaling_method);

template<typename T>
MAPNIK_DECL void scale_image_agg(T& target,
                                 T const& source,
                                 scaling_method_e scaling_method,
                                 double image_ratio_x,
                                 double image_ratio_y,
                                 double x_off_f,
                                 double y_off_f,
                                 double filter_factor,
                                 std::optional<double> const& nodata_value);
template<typename T>
inline void scale_image_agg(T& target,
                            T const& source,
                            scaling_method_e scaling_method,
                            double image_ratio_x,
                            double image_ratio_y,
                            double x_off_f,
                            double y_off_f,
                            double filter_factor)
{
    scale_image_agg(target,
                    source,
                    scaling_method,
                    image_ratio_x,
                    image_ratio_y,
                    x_off_f,
                    y_off_f,
                    filter_factor,
                    std::optional<double>());
}
} // namespace mapnik

#endif // MAPNIK_IMAGE_SCALING_HPP
