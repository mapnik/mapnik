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
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/symbolizer_helpers.hpp>

// boost
#include <boost/make_shared.hpp>

namespace mapnik {

template <typename T>
void  agg_renderer<T>::process(shield_symbolizer const& sym,
                               mapnik::feature_ptr const& feature,
                               proj_transform const& prj_trans)
{
    shield_symbolizer_helper<face_manager<freetype_engine>,
        label_collision_detector4> helper(
            sym, *feature, prj_trans,
            width_, height_,
            scale_factor_,
            t_, font_manager_, *detector_, query_extent_);

    text_renderer<T> ren(pixmap_, font_manager_, *(font_manager_.get_stroker()));

    while (helper.next()) {
        placements_type &placements = helper.placements();
        for (unsigned int ii = 0; ii < placements.size(); ++ii)
        {
            render_marker(helper.get_marker_position(placements[ii]),
                          helper.get_marker(),
                          helper.get_image_transform(),
                          sym.get_opacity(),
                          sym.comp_op());

            ren.prepare_glyphs(&(placements[ii]));
            ren.render(placements[ii].center);
        }
    }
}


template void agg_renderer<image_32>::process(shield_symbolizer const&,
                                              mapnik::feature_ptr const&,
                                              proj_transform const&);

}
