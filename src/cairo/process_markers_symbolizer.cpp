/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#include <mapnik/cairo/cairo_render_vector.hpp>
#include <mapnik/markers_placement.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/renderer_common/process_markers_symbolizer.hpp>

// agg
#include "agg/include/agg_array.h"      // for pod_bvector
#include "agg/include/agg_trans_affine.h"  // for trans_affine, etc

namespace mapnik
{

class feature_impl;
class proj_transform;

namespace detail {

template <typename RendererContext, typename Detector>
struct vector_markers_dispatch_cairo  : public vector_markers_dispatch<Detector>
{
    vector_markers_dispatch_cairo(svg_path_ptr const& src,
                                  svg::svg_path_adapter & path,
                                  svg_attribute_type const& attrs,
                                  agg::trans_affine const& marker_trans,
                                  markers_symbolizer const& sym,
                                  Detector & detector,
                                  double scale_factor,
                                  feature_impl & feature,
                                  mapnik::attributes const& vars,
                                  bool snap_to_pixels,
                                  RendererContext const& renderer_context)
    : vector_markers_dispatch<Detector>(src, marker_trans, sym, detector, scale_factor, feature, vars),
        path_(path),
        attr_(attrs),
        ctx_(std::get<0>(renderer_context))
    {}

    void render_marker(agg::trans_affine const& marker_tr, double opacity)
    {
        render_vector_marker(ctx_,
                             path_,
                             attr_,
                             this->src_->bounding_box(),
                             marker_tr,
                             opacity);
    }

private:
    svg::svg_path_adapter & path_;
    svg_attribute_type const& attr_;
    cairo_context & ctx_;
};

template <typename RendererContext, typename Detector>
struct raster_markers_dispatch_cairo : public raster_markers_dispatch<Detector>
{
    raster_markers_dispatch_cairo(image_rgba8 const& src,
                                  agg::trans_affine const& marker_trans,
                                  markers_symbolizer const& sym,
                                  Detector & detector,
                                  double scale_factor,
                                  feature_impl & feature,
                                  mapnik::attributes const& vars,
                                  RendererContext const& renderer_context)
    : raster_markers_dispatch<Detector>(src, marker_trans, sym, detector, scale_factor, feature, vars),
        ctx_(std::get<0>(renderer_context)) {}

    ~raster_markers_dispatch_cairo() {}

    void render_marker(agg::trans_affine const& marker_tr, double opacity)
    {
        ctx_.add_image(marker_tr, this->src_, opacity);
    }

private:
    cairo_context & ctx_;
};

}

template <typename T>
void cairo_renderer<T>::process(markers_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    context_.set_operator(comp_op);
    box2d<double> clip_box = common_.query_extent_;

    auto renderer_context = std::tie(context_);

    using RendererContextType = decltype(renderer_context);
    using vector_dispatch_type = detail::vector_markers_dispatch_cairo<RendererContextType,
                                                                       label_collision_detector4>;

    using raster_dispatch_type = detail::raster_markers_dispatch_cairo<RendererContextType,
                                                                       label_collision_detector4>;


    render_markers_symbolizer<vector_dispatch_type, raster_dispatch_type>(
        sym, feature, prj_trans, common_, clip_box,
        renderer_context);
}

template void cairo_renderer<cairo_ptr>::process(markers_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}

#endif // HAVE_CAIRO
