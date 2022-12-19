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

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/text/symbolizer_helpers.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/renderer.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/renderer_common/clipping_extent.hpp>

namespace mapnik {

template<typename T0, typename T1>
void agg_renderer<T0, T1>::process(shield_symbolizer const& sym,
                                   mapnik::feature_impl& feature,
                                   proj_transform const& prj_trans)
{
    const box2d<double> clip_box = clipping_extent(common_);
    agg::trans_affine tr;
    const auto transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (transform)
        evaluate_transform(tr, feature, common_.vars_, *transform, common_.scale_factor_);
    const text_symbolizer_helper helper(sym,
                                        feature,
                                        common_.vars_,
                                        prj_trans,
                                        common_.width_,
                                        common_.height_,
                                        common_.scale_factor_,
                                        common_.t_,
                                        common_.font_manager_,
                                        *common_.detector_,
                                        clip_box,
                                        tr);

    const halo_rasterizer_enum halo_rasterizer = get<halo_rasterizer_enum>(sym,
                                                                           keys::halo_rasterizer,
                                                                           feature,
                                                                           common_.vars_,
                                                                           halo_rasterizer_enum::HALO_RASTERIZER_FULL);
    const composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    const composite_mode_e halo_comp_op =
      get<composite_mode_e>(sym, keys::halo_comp_op, feature, common_.vars_, src_over);
    agg_text_renderer<T0> ren(buffers_.top().get(),
                              halo_rasterizer,
                              comp_op,
                              halo_comp_op,
                              common_.scale_factor_,
                              common_.font_manager_.get_stroker());

    const double opacity = get<double>(sym, keys::opacity, feature, common_.vars_, 1.0);

    placements_list const& placements = helper.get();
    for (auto const& glyphs : placements)
    {
        const marker_info_ptr mark = glyphs->get_marker();
        if (mark)
        {
            render_marker(glyphs->marker_pos(), *mark->marker_, mark->transform_, opacity, comp_op);
        }
        ren.render(*glyphs);
    }
}

template void
  agg_renderer<image_rgba8>::process(shield_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik
