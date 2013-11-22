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
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/text/symbolizer_helpers.hpp>
#include <mapnik/text/renderer.hpp>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(text_symbolizer const& sym,
                               mapnik::feature_impl & feature,
                               proj_transform const& prj_trans)
{
    text_symbolizer_helper helper(
            sym, feature, prj_trans,
            width_, height_,
            scale_factor_ * (1.0/pixmap_.get_resolution()),
            t_, font_manager_, *detector_,
            query_extent_);

    grid_text_renderer<T> ren(pixmap_, sym.comp_op(), scale_factor_);

    placements_list const& placements = helper.get();
    if (!placements.size()) return;
    for (glyph_positions_ptr glyphs : placements)
    {
        ren.render(*glyphs, feature.id());
    }
    pixmap_.add_feature(feature);
}

template void grid_renderer<grid>::process(text_symbolizer const&,
                                           mapnik::feature_impl &,
                                           proj_transform const&);

}
