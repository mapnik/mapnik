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
//$Id$

// mapnik
#include <mapnik/grid/grid_renderer.hpp>
#include <iostream>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(glyph_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    std::clog << "grid_renderer does not yet support glyph_symbolizer\n";
}

template void grid_renderer<grid>::process(glyph_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);
}
