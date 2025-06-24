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

#if defined(HAVE_CAIRO)

#ifndef MAPNIK_CAIRO_RENDER_VECTOR_HPP
#define MAPNIK_CAIRO_RENDER_VECTOR_HPP

// mapnik
#include <mapnik/marker.hpp>

namespace mapnik {

class cairo_context;

void render_vector_marker(cairo_context& context,
                          svg_path_adapter& svg_path,
                          svg::group const& group_attr,
                          box2d<double> const& bbox,
                          agg::trans_affine const& tr,
                          double opacity);

} // namespace mapnik

#endif // MAPNIK_CAIRO_RENDER_VECTOR_HPP

#endif
