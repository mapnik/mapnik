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
#include <mapnik/config.hpp>
#include <mapnik/image.hpp>
#include <mapnik/image_impl.hpp>
#include <mapnik/pixel_types.hpp>

namespace mapnik 
{

template class MAPNIK_DECL image<null_t>;
template class MAPNIK_DECL image<rgba8_t>;
template class MAPNIK_DECL image<gray8_t>;
template class MAPNIK_DECL image<gray8s_t>;
template class MAPNIK_DECL image<gray16_t>;
template class MAPNIK_DECL image<gray16s_t>;
template class MAPNIK_DECL image<gray32_t>;
template class MAPNIK_DECL image<gray32s_t>;
template class MAPNIK_DECL image<gray32f_t>;
template class MAPNIK_DECL image<gray64_t>;
template class MAPNIK_DECL image<gray64s_t>;
template class MAPNIK_DECL image<gray64f_t>;

} // end ns
