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

#if defined(SVG_RENDERER)

// mapnik
#include <mapnik/svg/output/svg_renderer.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/symbolizer.hpp>

namespace mapnik
{
/*!
 * @brief Collect presentation attributes found in polygon symbolizer.
 */
template <typename T>
void svg_renderer<T>::process(polygon_symbolizer const& sym,
                              mapnik::feature_impl &,
                              proj_transform const&)
{
    path_attributes_.set_fill_color(get<mapnik::color>(sym, keys::fill, mapnik::color(128,128,128)));
    path_attributes_.set_fill_opacity(get<value_double>(sym,keys::fill_opacity, 1.0));
}

template void svg_renderer<std::ostream_iterator<char> >::process(polygon_symbolizer const& sym,
                                                                  mapnik::feature_impl & feature,
                                                                  proj_transform const& prj_trans);
}

#endif
