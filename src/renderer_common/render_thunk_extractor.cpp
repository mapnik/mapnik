/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2016 Artem Pavlenko
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
#include <mapnik/make_unique.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/renderer_common/process_markers_symbolizer.hpp>
#include <mapnik/renderer_common/render_thunk_extractor.hpp>

namespace mapnik {

virtual_renderer_common::virtual_renderer_common(renderer_common const& other)
    : renderer_common(other)
{
    // replace collision detector with my own so that I don't pollute the original
    detector_ = std::make_shared<label_collision_detector4>(other.detector_->extent());
}

namespace detail {

template <typename Detector, typename RendererContext>
struct vector_marker_thunk_dispatch : public vector_markers_dispatch<Detector>
{
    vector_marker_thunk_dispatch(svg_path_ptr const& src,
                                 svg_path_adapter & path,
                                 svg_attribute_type const& attrs,
                                 agg::trans_affine const& marker_trans,
                                 symbolizer_base const& sym,
                                 Detector & detector,
                                 double scale_factor,
                                 feature_impl & feature,
                                 attributes const& vars,
                                 bool snap_to_pixels,
                                 RendererContext const& renderer_context)
        : vector_markers_dispatch<Detector>(src, marker_trans, sym, detector, scale_factor, feature, vars),
          attrs_(attrs), comp_op_(get<composite_mode_e, keys::comp_op>(sym, feature, vars)),
          snap_to_pixels_(snap_to_pixels), thunks_(std::get<0>(renderer_context))
    {}

    ~vector_marker_thunk_dispatch() {}

    void render_marker(agg::trans_affine const& marker_tr, double opacity)
    {
        vector_marker_render_thunk thunk(this->src_, this->attrs_, marker_tr, opacity, comp_op_, snap_to_pixels_);
        thunks_.push_back(std::make_unique<render_thunk>(std::move(thunk)));
    }

private:
    svg_attribute_type const& attrs_;
    composite_mode_e comp_op_;
    bool snap_to_pixels_;
    render_thunk_list & thunks_;
};

template <typename Detector, typename RendererContext>
struct raster_marker_thunk_dispatch : public raster_markers_dispatch<Detector>
{
    raster_marker_thunk_dispatch(image_rgba8 const& src,
                                 agg::trans_affine const& marker_trans,
                                 symbolizer_base const& sym,
                                 Detector & detector,
                                 double scale_factor,
                                 feature_impl & feature,
                                 attributes const& vars,
                                 RendererContext const& renderer_context,
                                 bool snap_to_pixels = false)
        : raster_markers_dispatch<Detector>(src, marker_trans, sym, detector, scale_factor, feature, vars),
          comp_op_(get<composite_mode_e, keys::comp_op>(sym, feature, vars)),
          snap_to_pixels_(snap_to_pixels), thunks_(std::get<0>(renderer_context))
    {}

    ~raster_marker_thunk_dispatch() {}

    void render_marker(agg::trans_affine const& marker_tr, double opacity)
    {
        raster_marker_render_thunk thunk(this->src_, marker_tr, opacity, comp_op_, snap_to_pixels_);
        thunks_.push_back(std::make_unique<render_thunk>(std::move(thunk)));
    }

private:
    composite_mode_e comp_op_;
    bool snap_to_pixels_;
    render_thunk_list & thunks_;
};

} // namespace detail

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

void render_thunk_extractor::operator()(markers_symbolizer const& sym) const
{
    auto renderer_context = std::tie(thunks_);
    using thunk_context_type = decltype(renderer_context);
    using vector_dispatch_type = detail::vector_marker_thunk_dispatch<label_collision_detector4, thunk_context_type>;
    using raster_dispatch_type = detail::raster_marker_thunk_dispatch<label_collision_detector4, thunk_context_type>;

    render_markers_symbolizer<vector_dispatch_type, raster_dispatch_type>(
            sym, feature_, prj_trans_, common_, clipping_extent_, renderer_context);

    update_box();
}

void render_thunk_extractor::operator()(text_symbolizer const& sym) const
{
    extract_text_thunk(sym);
}

void render_thunk_extractor::operator()(shield_symbolizer const& sym) const
{
    extract_text_thunk(sym);
}

void render_thunk_extractor::extract_text_thunk(text_symbolizer const& sym) const
{
    auto helper = std::make_unique<text_symbolizer_helper>(
        sym, feature_, vars_, prj_trans_,
        common_.width_, common_.height_,
        common_.scale_factor_,
        common_.t_, common_.font_manager_, *common_.detector_,
        clipping_extent_, agg::trans_affine::identity);

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
