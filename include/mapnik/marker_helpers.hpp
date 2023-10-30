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

#ifndef MAPNIK_MARKER_HELPERS_HPP
#define MAPNIK_MARKER_HELPERS_HPP

#include <mapnik/feature.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry/geometry_type.hpp>
#include <mapnik/geometry/centroid.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/marker.hpp> // svg_attribute_type, svg_storage_type
#include <mapnik/markers_placement.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>
#include <mapnik/renderer_common/render_markers_symbolizer.hpp>
#include <mapnik/vertex_converters.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_trans_affine.h"
MAPNIK_DISABLE_WARNING_POP

// stl
#include <memory>

namespace mapnik {

struct clip_poly_tag;

template<typename Detector>
struct vector_markers_dispatch : util::noncopyable
{
    vector_markers_dispatch(svg_path_ptr const& src,
                            svg_path_adapter& path,
                            svg_attribute_type const& attrs,
                            agg::trans_affine const& marker_trans,
                            symbolizer_base const& sym,
                            Detector& detector,
                            double scale_factor,
                            feature_impl const& feature,
                            attributes const& vars,
                            bool snap_to_pixels,
                            markers_renderer_context& renderer_context)
        : params_(src->bounding_box(), recenter(src) * marker_trans, sym, feature, vars, scale_factor, snap_to_pixels)
        , renderer_context_(renderer_context)
        , src_(src)
        , path_(path)
        , attrs_(attrs)
        , detector_(detector)
    {}

    template<typename T>
    void add_path(T& path)
    {
        markers_placement_finder<T, Detector> placement_finder(params_.placement_method,
                                                               path,
                                                               detector_,
                                                               params_.placement_params);
        double x, y, angle = .0;
        while (placement_finder.get_point(x, y, angle, params_.ignore_placement))
        {
            agg::trans_affine matrix = params_.placement_params.tr;
            matrix.rotate(angle);
            matrix.translate(x, y);
            renderer_context_.render_marker(src_, path_, attrs_, params_, matrix);
        }
    }

  protected:
    static agg::trans_affine recenter(svg_path_ptr const& src)
    {
        coord2d center = src->bounding_box().center();
        return agg::trans_affine_translation(-center.x, -center.y);
    }

    markers_dispatch_params params_;
    markers_renderer_context& renderer_context_;
    svg_path_ptr const& src_;
    svg_path_adapter& path_;
    svg_attribute_type const& attrs_;
    Detector& detector_;
};

template<typename Detector>
struct raster_markers_dispatch : util::noncopyable
{
    raster_markers_dispatch(image_rgba8 const& src,
                            agg::trans_affine const& marker_trans,
                            symbolizer_base const& sym,
                            Detector& detector,
                            double scale_factor,
                            feature_impl const& feature,
                            attributes const& vars,
                            markers_renderer_context& renderer_context)
        : params_(box2d<double>(0, 0, src.width(), src.height()), marker_trans, sym, feature, vars, scale_factor)
        , renderer_context_(renderer_context)
        , src_(src)
        , detector_(detector)
    {}

    template<typename T>
    void add_path(T& path)
    {
        markers_placement_finder<T, Detector> placement_finder(params_.placement_method,
                                                               path,
                                                               detector_,
                                                               params_.placement_params);
        double x, y, angle = .0;
        while (placement_finder.get_point(x, y, angle, params_.ignore_placement))
        {
            agg::trans_affine matrix = params_.placement_params.tr;
            matrix.rotate(angle);
            matrix.translate(x, y);
            renderer_context_.render_marker(src_, params_, matrix);
        }
    }

  protected:
    markers_dispatch_params params_;
    markers_renderer_context& renderer_context_;
    image_rgba8 const& src_;
    Detector& detector_;
};

void build_ellipse(symbolizer_base const& sym,
                   mapnik::feature_impl& feature,
                   attributes const& vars,
                   svg_storage_type& marker_ellipse,
                   svg::svg_path_adapter& svg_path);

bool push_explicit_style(svg_attribute_type const& src,
                         svg_attribute_type& dst,
                         symbolizer_base const& sym,
                         feature_impl& feature,
                         attributes const& vars);

void setup_transform_scaling(agg::trans_affine& tr,
                             double svg_width,
                             double svg_height,
                             mapnik::feature_impl& feature,
                             attributes const& vars,
                             symbolizer_base const& sym);

using vertex_converter_type = vertex_converter<clip_line_tag,
                                               clip_poly_tag,
                                               transform_tag,
                                               affine_transform_tag,
                                               simplify_tag,
                                               smooth_tag,
                                               offset_transform_tag>;

// Apply markers to a feature with multiple geometries
template<typename Processor>
void apply_markers_multi(feature_impl const& feature,
                         attributes const& vars,
                         vertex_converter_type& converter,
                         Processor& proc,
                         symbolizer_base const& sym);

using vector_dispatch_type = vector_markers_dispatch<mapnik::label_collision_detector4>;
using raster_dispatch_type = raster_markers_dispatch<mapnik::label_collision_detector4>;

extern template void apply_markers_multi<vector_dispatch_type>(feature_impl const& feature,
                                                               attributes const& vars,
                                                               vertex_converter_type& converter,
                                                               vector_dispatch_type& proc,
                                                               symbolizer_base const& sym);

extern template void apply_markers_multi<raster_dispatch_type>(feature_impl const& feature,
                                                               attributes const& vars,
                                                               vertex_converter_type& converter,
                                                               raster_dispatch_type& proc,
                                                               symbolizer_base const& sym);

} // namespace mapnik

#endif // MAPNIK_MARKER_HELPERS_HPP
