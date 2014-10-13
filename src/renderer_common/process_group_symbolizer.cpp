/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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
#include <mapnik/renderer_common/process_group_symbolizer.hpp>
#include <mapnik/renderer_common/process_point_symbolizer.hpp>
#include <mapnik/text/glyph_info.hpp>
#include <mapnik/make_unique.hpp>

namespace mapnik {

point_render_thunk::point_render_thunk(pixel_position const& pos, marker const& m,
                                       agg::trans_affine const& tr, double opacity,
                                       composite_mode_e comp_op)
    : pos_(pos), marker_(std::make_shared<marker>(m)),
      tr_(tr), opacity_(opacity), comp_op_(comp_op)
{}


text_render_thunk::text_render_thunk(helper_ptr && helper,
                                     double opacity, composite_mode_e comp_op,
                                     halo_rasterizer_enum halo_rasterizer)
    : helper_(std::move(helper)),
      placements_(helper_->get()),
      opacity_(opacity),
      comp_op_(comp_op),
      halo_rasterizer_(halo_rasterizer) {}

render_thunk_extractor::render_thunk_extractor(box2d<double> & box,
                                               render_thunk_list & thunks,
                                               feature_impl & feature,
                                               attributes const& vars,
                                               proj_transform const& prj_trans,
                                               virtual_renderer_common & common,
                                               box2d<double> const& clipping_extent)
    : box_(box), thunks_(thunks), feature_(feature), vars_(vars), prj_trans_(prj_trans),
      common_(common), clipping_extent_(clipping_extent)
{}

void render_thunk_extractor::operator()(point_symbolizer const& sym) const
{
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature_, common_.vars_, src_over);

    render_point_symbolizer(
        sym, feature_, prj_trans_, common_,
        [&](pixel_position const& pos, marker const& marker,
            agg::trans_affine const& tr, double opacity) {
            point_render_thunk thunk(pos, marker, tr, opacity, comp_op);
            thunks_.push_back(std::make_unique<render_thunk>(std::move(thunk)));
        });

    update_box();
}

void render_thunk_extractor::operator()(text_symbolizer const& sym) const
{
    box2d<double> clip_box = clipping_extent_;
    helper_ptr helper = std::make_unique<text_symbolizer_helper>(
        sym, feature_, vars_, prj_trans_,
        common_.width_, common_.height_,
        common_.scale_factor_,
        common_.t_, common_.font_manager_, *common_.detector_,
        clip_box, agg::trans_affine());

    extract_text_thunk(std::move(helper), sym);
}

void render_thunk_extractor::operator()(shield_symbolizer const& sym) const
{
    box2d<double> clip_box = clipping_extent_;
    helper_ptr helper = std::make_unique<text_symbolizer_helper>(
        sym, feature_, vars_, prj_trans_,
        common_.width_, common_.height_,
        common_.scale_factor_,
        common_.t_, common_.font_manager_, *common_.detector_,
        clip_box, agg::trans_affine());

    extract_text_thunk(std::move(helper), sym);
}

void render_thunk_extractor::extract_text_thunk(helper_ptr && helper, text_symbolizer const& sym) const
{
    double opacity = get<double>(sym, keys::opacity, feature_, common_.vars_, 1.0);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature_, common_.vars_, src_over);
    halo_rasterizer_enum halo_rasterizer = get<halo_rasterizer_enum>(sym, keys::halo_rasterizer, feature_, common_.vars_, HALO_RASTERIZER_FULL);

    text_render_thunk thunk(std::move(helper), opacity, comp_op, halo_rasterizer);
    thunks_.push_back(std::make_unique<render_thunk>(std::move(thunk)));

    update_box();
}

void render_thunk_extractor::update_box() const
{
    label_collision_detector4 & detector = *common_.detector_;

    for (auto const& label : detector)
    {
        if (box_.width() > 0 && box_.height() > 0)
        {
            box_.expand_to_include(label.box);
        }
        else
        {
            box_ = label.box;
        }
    }

    detector.clear();
}


geometry_type *origin_point(proj_transform const& prj_trans,
                            renderer_common const& common)
{
    // note that we choose a point in the middle of the screen to
    // try to ensure that we don't get edge artefacts due to any
    // symbolizers with avoid-edges set: only the avoid-edges of
    // the group symbolizer itself should matter.
    double x = common.width_ / 2.0, y = common.height_ / 2.0, z = 0.0;
    common.t_.backward(&x, &y);
    prj_trans.forward(x, y, z);
    geometry_type *geom = new geometry_type(geometry_type::Point);
    geom->move_to(x, y);
    return geom;
}

} // namespace mapnik
