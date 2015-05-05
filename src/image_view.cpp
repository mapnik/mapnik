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

// mapnik
#include <mapnik/image_view.hpp>
#include <mapnik/image_view_impl.hpp>
#include <mapnik/image_view_null.hpp>
#include <mapnik/pixel_types.hpp>

namespace mapnik
{

template class MAPNIK_DECL image_view<image_null>;
template class MAPNIK_DECL image_view<image_rgba8>;
template class MAPNIK_DECL image_view<image_gray8>;
template class MAPNIK_DECL image_view<image_gray8s>;
template class MAPNIK_DECL image_view<image_gray16>;
template class MAPNIK_DECL image_view<image_gray16s>;
template class MAPNIK_DECL image_view<image_gray32>;
template class MAPNIK_DECL image_view<image_gray32s>;
template class MAPNIK_DECL image_view<image_gray32f>;
template class MAPNIK_DECL image_view<image_gray64>;
template class MAPNIK_DECL image_view<image_gray64s>;
template class MAPNIK_DECL image_view<image_gray64f>;

} // end ns mapnik
