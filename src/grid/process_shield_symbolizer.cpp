/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#if defined(GRID_RENDERER)

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/grid/grid_rasterizer.hpp>
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/grid/grid_renderer_base.hpp>
#include <mapnik/grid/grid.hpp>
#include <mapnik/text/symbolizer_helpers.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/renderer.hpp>
#include <mapnik/text/glyph_positions.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_trans_affine.h"
#include "agg_gamma_functions.h"
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {

template<typename T>
void grid_renderer<T>::process(shield_symbolizer const& sym,
                               mapnik::feature_impl& feature,
                               proj_transform const& prj_trans)
{
    agg::trans_affine tr;
    auto transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (transform)
        evaluate_transform(tr, feature, common_.vars_, *transform, common_.scale_factor_);

    text_symbolizer_helper helper(sym,
                                  feature,
                                  common_.vars_,
                                  prj_trans,
                                  common_.width_,
                                  common_.height_,
                                  common_.scale_factor_,
                                  common_.t_,
                                  common_.font_manager_,
                                  *common_.detector_,
                                  common_.query_extent_,
                                  tr);
    bool placement_found = false;

    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    double opacity = get<double>(sym, keys::opacity, feature, common_.vars_, 1.0);

    grid_text_renderer<T> ren(pixmap_, comp_op, common_.scale_factor_);

    placements_list const& placements = helper.get();
    value_integer feature_id = feature.id();

    for (auto const& glyphs : placements)
    {
        marker_info_ptr mark = glyphs->get_marker();
        if (mark)
        {
            render_marker(feature, glyphs->marker_pos(), *mark->marker_, mark->transform_, opacity, comp_op);
        }
        ren.render(*glyphs, feature_id);
        placement_found = true;
    }
    if (placement_found)
    {
        pixmap_.add_feature(feature);
    }
}

template void grid_renderer<grid>::process(shield_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik

#endif
