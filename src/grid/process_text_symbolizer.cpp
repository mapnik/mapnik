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
#include <mapnik/symbolizer_helpers.hpp>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(text_symbolizer const& sym,
                               mapnik::feature_ptr const& feature,
                               proj_transform const& prj_trans)
{
    box2d<double> query_extent;
    text_symbolizer_helper<face_manager<freetype_engine>,
        label_collision_detector4> helper(
            sym, *feature, prj_trans,
            width_, height_,
            scale_factor_ * (1.0/pixmap_.get_resolution()),
            t_, font_manager_, detector_,
            query_extent);
    bool placement_found = false;

    text_renderer<T> ren(pixmap_, font_manager_, *(font_manager_.get_stroker()));

    while (helper.next()) {
        placement_found = true;
        placements_type &placements = helper.placements();
        for (unsigned int ii = 0; ii < placements.size(); ++ii)
        {
            ren.prepare_glyphs(&(placements[ii]));
            ren.render_id(feature->id(), placements[ii].center, 2);
        }
    }
    if (placement_found) pixmap_.add_feature(feature);

}

template void grid_renderer<grid>::process(text_symbolizer const&,
                                           mapnik::feature_ptr const&,
                                           proj_transform const&);

}

