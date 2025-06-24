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

#if defined(GRID_RENDERER)

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/grid/grid_renderer.hpp>

namespace mapnik {

template<typename T>
void grid_renderer<T>::process(raster_symbolizer const&, mapnik::feature_impl&, proj_transform const&)
{
    MAPNIK_LOG_WARN(grid_renderer) << "grid_renderer: raster_symbolizer is not yet supported";
}

template void grid_renderer<grid>::process(raster_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik

#endif
