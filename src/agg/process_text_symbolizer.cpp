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
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/text/symbolizer_helpers.hpp>
#include <mapnik/text/renderer.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>

namespace mapnik {

template <typename T0, typename T1>
void agg_renderer<T0,T1>::process(text_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{

    box2d<double> clip_box = clipping_extent(common_);
    agg::trans_affine tr;
    auto transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (transform) evaluate_transform(tr, feature, common_.vars_, *transform, common_.scale_factor_);
    text_symbolizer_helper helper(
        sym, feature, common_.vars_, prj_trans,
        common_.width_, common_.height_,
        common_.scale_factor_,
        common_.t_, common_.font_manager_, *common_.detector_,
        clip_box, tr);

    halo_rasterizer_enum halo_rasterizer = get<halo_rasterizer_enum>(sym, keys::halo_rasterizer,feature, common_.vars_, HALO_RASTERIZER_FULL);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    composite_mode_e halo_comp_op = get<composite_mode_e>(sym, keys::halo_comp_op, feature, common_.vars_, src_over);
    agg_text_renderer<T0> ren(*current_buffer_,
                              halo_rasterizer,
                              comp_op,
                              halo_comp_op,
                              common_.scale_factor_,
                              common_.font_manager_.get_stroker());

    { // halo transform
        agg::trans_affine halo_transform;
        auto transform = get_optional<transform_type>(sym, keys::halo_transform);
        if (transform)
        {
            evaluate_transform(halo_transform, feature, common_.vars_, *transform);
            ren.set_halo_transform(halo_transform);
        }
    }

    placements_list const& placements = helper.get();
    for (glyph_positions_ptr glyphs : placements)
    {
        ren.render(*glyphs);
    }
}

template void agg_renderer<image_32>::process(text_symbolizer const&,
                                              mapnik::feature_impl &,
                                              proj_transform const&);

}
