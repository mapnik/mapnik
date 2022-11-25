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
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/renderer_common/render_markers_symbolizer.hpp>
#include <mapnik/renderer_common/render_thunk_extractor.hpp>

namespace mapnik {

virtual_renderer_common::virtual_renderer_common(renderer_common const& other)
    : renderer_common(other)
{
    // replace collision detector with my own so that I don't pollute the original
    detector_ = std::make_shared<label_collision_detector4>(other.detector_->extent());
}

namespace detail {

struct thunk_markers_renderer_context : markers_renderer_context
{
    thunk_markers_renderer_context(symbolizer_base const& sym,
                                   feature_impl const& feature,
                                   attributes const& vars,
                                   render_thunk_list& thunks)
        : comp_op_(get<composite_mode_e, keys::comp_op>(sym, feature, vars))
        , thunks_(thunks)
    {}

    virtual void render_marker(svg_path_ptr const& src,
                               svg_path_adapter& path,
                               svg::group const& group_attrs,
                               markers_dispatch_params const& params,
                               agg::trans_affine const& marker_tr)
    {
        vector_marker_render_thunk thunk(src, group_attrs, marker_tr, params.opacity, comp_op_, params.snap_to_pixels);
        thunks_.emplace_back(std::move(thunk));
    }

    virtual void
      render_marker(image_rgba8 const& src, markers_dispatch_params const& params, agg::trans_affine const& marker_tr)
    {
        raster_marker_render_thunk thunk(src, marker_tr, params.opacity, comp_op_, params.snap_to_pixels);
        thunks_.emplace_back(std::move(thunk));
    }

  private:
    composite_mode_e comp_op_;
    render_thunk_list& thunks_;
};

} // namespace detail

render_thunk_extractor::render_thunk_extractor(box2d<double>& box,
                                               render_thunk_list& thunks,
                                               feature_impl& feature,
                                               attributes const& vars,
                                               proj_transform const& prj_trans,
                                               virtual_renderer_common& common,
                                               box2d<double> const& clipping_extent)
    : box_(box)
    , thunks_(thunks)
    , feature_(feature)
    , vars_(vars)
    , prj_trans_(prj_trans)
    , common_(common)
    , clipping_extent_(clipping_extent)
{}

void render_thunk_extractor::operator()(markers_symbolizer const& sym) const
{
    using renderer_context_type = detail::thunk_markers_renderer_context;
    renderer_context_type renderer_context(sym, feature_, vars_, thunks_);

    render_markers_symbolizer(sym, feature_, prj_trans_, common_, clipping_extent_, renderer_context);

    update_box();
}

void render_thunk_extractor::operator()(text_symbolizer const& sym) const
{
    auto helper = std::make_unique<text_symbolizer_helper>(sym,
                                                           feature_,
                                                           vars_,
                                                           prj_trans_,
                                                           common_.width_,
                                                           common_.height_,
                                                           common_.scale_factor_,
                                                           common_.t_,
                                                           common_.font_manager_,
                                                           *common_.detector_,
                                                           clipping_extent_,
                                                           agg::trans_affine::identity);

    extract_text_thunk(std::move(helper), sym);
}

void render_thunk_extractor::operator()(shield_symbolizer const& sym) const
{
    auto helper = std::make_unique<text_symbolizer_helper>(sym,
                                                           feature_,
                                                           vars_,
                                                           prj_trans_,
                                                           common_.width_,
                                                           common_.height_,
                                                           common_.scale_factor_,
                                                           common_.t_,
                                                           common_.font_manager_,
                                                           *common_.detector_,
                                                           clipping_extent_,
                                                           agg::trans_affine::identity);

    extract_text_thunk(std::move(helper), sym);
}

void render_thunk_extractor::extract_text_thunk(text_render_thunk::helper_ptr&& helper,
                                                text_symbolizer const& sym) const
{
    double opacity = get<double>(sym, keys::opacity, feature_, common_.vars_, 1.0);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature_, common_.vars_, src_over);
    halo_rasterizer_enum halo_rasterizer =
      get<halo_rasterizer_enum>(sym, keys::halo_rasterizer, feature_, common_.vars_, HALO_RASTERIZER_FULL);

    text_render_thunk thunk(std::move(helper), opacity, comp_op, halo_rasterizer);
    thunks_.emplace_back(std::move(thunk));

    update_box();
}

void render_thunk_extractor::update_box() const
{
    label_collision_detector4& detector = *common_.detector_;

    for (auto const& label : detector)
    {
        if (box_.width() > 0 && box_.height() > 0)
        {
            box_.expand_to_include(label.get().box);
        }
        else
        {
            box_ = label.get().box;
        }
    }

    detector.clear();
}

} // namespace mapnik
