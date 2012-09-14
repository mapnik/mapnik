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
#include <mapnik/svg/output/svg_renderer.hpp>

namespace mapnik
{
/*!
 * @brief Collect presentation attributes found in line symbolizer.
 */
template <typename T>
void svg_renderer<T>::process(line_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    path_attributes_.set_stroke_color(sym.get_stroke().get_color());
    path_attributes_.set_stroke_opacity(sym.get_stroke().get_opacity());
    path_attributes_.set_stroke_width(sym.get_stroke().get_width());
    path_attributes_.set_stroke_linecap(sym.get_stroke().get_line_cap());
    path_attributes_.set_stroke_linejoin(sym.get_stroke().get_line_join());
    path_attributes_.set_stroke_dasharray(sym.get_stroke().get_dash_array());
    path_attributes_.set_stroke_dashoffset(sym.get_stroke().dash_offset());
}

template void svg_renderer<std::ostream_iterator<char> >::process(line_symbolizer const& sym,
                                                                  Feature const& feature,
                                                                  proj_transform const& prj_trans);
}
