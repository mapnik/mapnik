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
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/symbolizer_helpers.hpp>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(text_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    text_symbolizer_helper<face_manager<freetype_engine>,
            label_collision_detector4> helper(
                width_, height_,
                scale_factor_ * (1.0/pixmap_.get_resolution()),
                t_, font_manager_, detector_);

    text_placement_info_ptr placement = helper.get_placement(sym, feature, prj_trans);

    if (!placement) return;

    text_renderer<T> ren(pixmap_, font_manager_, *(font_manager_.get_stroker()));
    for (unsigned int ii = 0; ii < placement->placements.size(); ++ii)
    {
        double x = placement->placements[ii].starting_x;
        double y = placement->placements[ii].starting_y;
        ren.prepare_glyphs(&(placement->placements[ii]));
        ren.render_id(feature.id(),x,y,2);
    }
    pixmap_.add_feature(feature);

}

template void grid_renderer<grid>::process(text_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
 
