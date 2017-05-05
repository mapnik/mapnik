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
 * @brief Collect presentation attributes found in line symbolizer.
 */
template <typename T>
void svg_renderer<T>::process(line_symbolizer const& sym,
                              mapnik::feature_impl & /*feature*/,
                              proj_transform const& /*prj_trans*/)
{
    path_attributes_.set_stroke_color(get<color>(sym, keys::stroke, mapnik::color(0,0,0)));
    path_attributes_.set_stroke_opacity(get<value_double>(sym,keys::stroke_opacity, 1.0));
    path_attributes_.set_stroke_width(get<value_double>(sym, keys::stroke_width, 1.0));
    /*
    path_attributes_.set_stroke_linecap(sym.get_stroke().get_line_cap());
    path_attributes_.set_stroke_linejoin(sym.get_stroke().get_line_join());
    path_attributes_.set_stroke_dasharray(sym.get_stroke().get_dash_array());
    path_attributes_.set_stroke_dashoffset(sym.get_stroke().dash_offset());
    */
}

template void svg_renderer<std::ostream_iterator<char> >::process(line_symbolizer const& sym,
                                                                  mapnik::feature_impl & feature,
                                                                  proj_transform const& prj_trans);
}

#endif
