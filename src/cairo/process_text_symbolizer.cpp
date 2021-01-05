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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/text/symbolizer_helpers.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/text/glyph_positions.hpp>

namespace mapnik
{

class feature_impl;
class proj_transform;

template <typename T>
void cairo_renderer<T>::process(shield_symbolizer const& sym,
                                mapnik::feature_impl & feature,
                                proj_transform const& prj_trans)
{
    agg::trans_affine tr;
    auto transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (transform) evaluate_transform(tr, feature, common_.vars_, *transform, common_.scale_factor_);
    text_symbolizer_helper helper(
            sym, feature, common_.vars_, prj_trans,
            common_.width_, common_.height_,
            common_.scale_factor_,
            common_.t_, common_.font_manager_, *common_.detector_,
            common_.query_extent_, tr);

    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    composite_mode_e halo_comp_op = get<composite_mode_e>(sym, keys::halo_comp_op, feature, common_.vars_, src_over);
    double opacity = get<double>(sym,keys::opacity,feature, common_.vars_, 1.0);

    placements_list const &placements = helper.get();
    for (auto const& glyphs : placements)
    {
        marker_info_ptr mark = glyphs->get_marker();
        if (mark) {
            pixel_position pos = glyphs->marker_pos();
            render_marker(pos,
                          *mark->marker_,
                          mark->transform_,
                          opacity);
        }
        context_.add_text(*glyphs, face_manager_, comp_op, halo_comp_op, common_.scale_factor_);
    }
}

template void cairo_renderer<cairo_ptr>::process(shield_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

template <typename T>
void cairo_renderer<T>::process(text_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    agg::trans_affine tr;
    auto transform = get_optional<transform_type>(sym, keys::geometry_transform);
    if (transform) evaluate_transform(tr, feature, common_.vars_, *transform, common_.scale_factor_);
    text_symbolizer_helper helper(
            sym, feature, common_.vars_, prj_trans,
            common_.width_, common_.height_,
            common_.scale_factor_,
            common_.t_, common_.font_manager_, *common_.detector_,
            common_.query_extent_, tr);

    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_,  src_over);
    composite_mode_e halo_comp_op = get<composite_mode_e>(sym, keys::halo_comp_op, feature, common_.vars_,  src_over);

    placements_list const& placements = helper.get();
    for (auto const& glyphs : placements)
    {
        context_.add_text(*glyphs, face_manager_, comp_op, halo_comp_op, common_.scale_factor_);
    }
}

template void cairo_renderer<cairo_ptr>::process(text_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
