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

#ifndef MAPNIK_WARP_HPP
#define MAPNIK_WARP_HPP

// mapnik
#include <mapnik/image_scaling.hpp>
#include <mapnik/config.hpp>
#include <mapnik/box2d.hpp>

namespace mapnik {

class raster;
class proj_transform;

MAPNIK_DECL void reproject_and_scale_raster(raster & target,
                                            raster const& source,
                                            proj_transform const& prj_trans,
                                            double offset_x, double offset_y,
                                            unsigned mesh_size,
                                            scaling_method_e scaling_method);


template <typename T>
MAPNIK_DECL void warp_image (T & target, T const& source, proj_transform const& prj_trans,
                             box2d<double> const& target_ext, box2d<double> const& source_ext,
                             double offset_x, double offset_y, unsigned mesh_size, scaling_method_e scaling_method, double filter_factor);
}

#endif // MAPNIK_WARP_HPP
