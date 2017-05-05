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

#ifndef MAPNIK_IMAGE_COPY_HPP
#define MAPNIK_IMAGE_COPY_HPP

#include <mapnik/image_any.hpp>
#include <mapnik/config.hpp>

namespace mapnik
{

template <typename T>
MAPNIK_DECL T image_copy(image_any const&, double offset = 0.0, double scaling = 1.0);

template <typename T>
MAPNIK_DECL T image_copy(image_rgba8 const&, double offset = 0.0, double scaling = 1.0);

template <typename T>
MAPNIK_DECL T image_copy(image_gray8 const&, double offset = 0.0, double scaling = 1.0);

template <typename T>
MAPNIK_DECL T image_copy(image_gray8s const&, double offset = 0.0, double scaling = 1.0);

template <typename T>
MAPNIK_DECL T image_copy(image_gray16 const&, double offset = 0.0, double scaling = 1.0);

template <typename T>
MAPNIK_DECL T image_copy(image_gray16s const&, double offset = 0.0, double scaling = 1.0);

template <typename T>
MAPNIK_DECL T image_copy(image_gray32 const&, double offset = 0.0, double scaling = 1.0);

template <typename T>
MAPNIK_DECL T image_copy(image_gray32s const&, double offset = 0.0, double scaling = 1.0);

template <typename T>
MAPNIK_DECL T image_copy(image_gray32f const&, double offset = 0.0, double scaling = 1.0);

template <typename T>
MAPNIK_DECL T image_copy(image_gray64 const&, double offset = 0.0, double scaling = 1.0);

template <typename T>
MAPNIK_DECL T image_copy(image_gray64s const&, double offset = 0.0, double scaling = 1.0);

template <typename T>
MAPNIK_DECL T image_copy(image_gray64f const&, double offset = 0.0, double scaling = 1.0);

MAPNIK_DECL image_any image_copy(image_any const&, image_dtype type, double offset = 0.0, double scaling = 1.0);

} // end mapnik ns

#endif // MAPNIK_IMAGE_COPY_HPP
