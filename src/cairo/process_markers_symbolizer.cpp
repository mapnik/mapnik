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
#include <mapnik/noncopyable.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/renderer_common/process_markers_symbolizer.hpp>

// agg
#include "agg/include/agg_array.h"      // for pod_bvector
#include "agg/include/agg_trans_affine.h"  // for trans_affine, etc

namespace mapnik
{

class feature_impl;
class proj_transform;
namespace svg { struct path_attributes; }

namespace detail {

template <typename RendererContext, typename SvgPath, typename Attributes, typename Detector>
struct markers_dispatch : mapnik::noncopyable
{
    markers_dispatch(SvgPath & marker,
                     Attributes const& attributes,
                     box2d<double> const& bbox,
                     agg::trans_affine const& marker_trans,
                     markers_symbolizer const& sym,
                     Detector & detector,
                     double scale_factor,
                     feature_impl const& feature,
                     mapnik::attributes const& vars,
                     bool snap_to_pixels,
                     RendererContext const& renderer_context)
        :marker_(marker),
        attributes_(attributes),
        bbox_(bbox),
        marker_trans_(marker_trans),
        sym_(sym),
        detector_(detector),
        scale_factor_(scale_factor),
        feature_(feature),
        vars_(vars),
        ctx_(std::get<0>(renderer_context))
    {}


    template <typename T>
    void add_path(T & path)
    {
        marker_placement_enum placement_method = get<marker_placement_enum>(
            sym_, keys::markers_placement_type, feature_, vars_, MARKER_POINT_PLACEMENT);
        value_bool ignore_placement = get<value_bool, keys::ignore_placement>(sym_, feature_, vars_);
        value_bool allow_overlap = get<value_bool, keys::allow_overlap>(sym_, feature_, vars_);
        value_bool avoid_edges = get<value_bool, keys::avoid_edges>(sym_, feature_, vars_);
        value_double opacity = get<value_double, keys::opacity>(sym_, feature_, vars_);
        value_double spacing = get<value_double, keys::spacing>(sym_, feature_, vars_);
        value_double max_error = get<value_double, keys::max_error>(sym_, feature_, vars_);
        coord2d center = bbox_.center();
        agg::trans_affine_translation recenter(-center.x, -center.y);
        agg::trans_affine tr = recenter * marker_trans_;
        markers_placement_params params { bbox_, tr, spacing * scale_factor_, max_error, allow_overlap, avoid_edges };
        markers_placement_finder<T, label_collision_detector4> placement_finder(
            placement_method, path, detector_, params);
        double x, y, angle = .0;
        while (placement_finder.get_point(x, y, angle, ignore_placement))
        {
            agg::trans_affine matrix = tr;
            matrix.rotate(angle);
            matrix.translate(x, y);
            render_vector_marker(
                ctx_,
                pixel_position(x, y),
                marker_,
                bbox_,
                attributes_,
                matrix,
                opacity,
                false);
        }
    }

    SvgPath & marker_;
    Attributes const& attributes_;
    box2d<double> const& bbox_;
    agg::trans_affine const& marker_trans_;
    markers_symbolizer const& sym_;
    Detector & detector_;
    double scale_factor_;
    feature_impl const& feature_;
    attributes const& vars_;
    cairo_context & ctx_;
};

template <typename RendererContext, typename Detector>
struct raster_markers_dispatch : mapnik::noncopyable
{
    raster_markers_dispatch(mapnik::image_data_32 & src,
                       agg::trans_affine const& marker_trans,
                       markers_symbolizer const& sym,
                       Detector & detector,
                       double scale_factor,
                       feature_impl const& feature,
                       mapnik::attributes const& vars,
                       RendererContext const& renderer_context)
        : src_(src),
        detector_(detector),
        sym_(sym),
        marker_trans_(marker_trans),
        scale_factor_(scale_factor),
        feature_(feature),
        vars_(vars),
        ctx_(std::get<0>(renderer_context)) {}

    template <typename T>
    void add_path(T & path)
    {
        marker_placement_enum placement_method = get<marker_placement_enum>(sym_, keys::markers_placement_type, feature_, vars_, MARKER_POINT_PLACEMENT);
        value_double opacity = get<value_double, keys::opacity>(sym_, feature_, vars_);
        value_double spacing = get<value_double, keys::spacing>(sym_, feature_, vars_);
        value_double max_error = get<value_double>(sym_, keys::max_error, feature_, vars_,  0.2); // overwrite default
        value_bool allow_overlap = get<value_bool, keys::allow_overlap>(sym_, feature_, vars_);
        value_bool avoid_edges = get<value_bool, keys::avoid_edges>(sym_, feature_, vars_);
        value_bool ignore_placement = get<value_bool, keys::ignore_placement>(sym_, feature_, vars_);
        box2d<double> bbox_(0,0, src_.width(),src_.height());
        markers_placement_params params { bbox_, marker_trans_, spacing * scale_factor_, max_error, allow_overlap, avoid_edges };
        markers_placement_finder<T, label_collision_detector4> placement_finder(
            placement_method, path, detector_, params);
        double x, y, angle = .0;
        while (placement_finder.get_point(x, y, angle, ignore_placement))
        {
            agg::trans_affine matrix = marker_trans_;
            matrix.rotate(angle);
            matrix.translate(x, y);
            ctx_.add_image(matrix, src_, opacity);
        }
    }

    image_data_32 & src_;
    Detector & detector_;
    markers_symbolizer const& sym_;
    agg::trans_affine const& marker_trans_;
    double scale_factor_;
    feature_impl const& feature_;
    attributes const& vars_;
    cairo_context & ctx_;
};

}

template <typename T>
void cairo_renderer<T>::process(markers_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    using svg_attribute_type = agg::pod_bvector<svg::path_attributes>;

    cairo_save_restore guard(context_);
    composite_mode_e comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);
    context_.set_operator(comp_op);
    box2d<double> clip_box = common_.query_extent_;

    auto renderer_context = std::tie(context_);

    using RendererContextType = decltype(renderer_context);
    using vector_dispatch_type = detail::markers_dispatch<RendererContextType,
                                                          svg::path_adapter<svg::vertex_stl_adapter<svg::svg_path_storage> >,
                                                          svg_attribute_type,label_collision_detector4>;

    using raster_dispatch_type = detail::raster_markers_dispatch<RendererContextType,
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
