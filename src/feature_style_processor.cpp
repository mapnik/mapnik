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

// mapnik

#include <mapnik/feature_style_processor_impl.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/graphics.hpp>

#if defined(GRID_RENDERER)
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid.hpp>
#endif

#if defined(HAVE_CAIRO)
#include <cairo.h>
#include <mapnik/cairo_renderer.hpp>
#endif

#if defined(SVG_RENDERER)
#include <mapnik/svg/output/svg_renderer.hpp>
#endif

namespace mapnik
{

#if defined(HAVE_CAIRO)
template class feature_style_processor<cairo_renderer<cairo_ptr> >;
template class feature_style_processor<cairo_renderer<cairo_surface_ptr> >;
#endif

#if defined(SVG_RENDERER)
template class feature_style_processor<svg_renderer<std::ostream_iterator<char> > >;
#endif

#if defined(GRID_RENDERER)
template class feature_style_processor<grid_renderer<grid> >;
#endif

template class feature_style_processor<agg_renderer<image_32> >;

}
