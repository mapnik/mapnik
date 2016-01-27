/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/geometry_type.hpp>
#include <mapnik/geometry_centroid.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/marker.hpp> // for svg_storage_type
#include <mapnik/markers_placement.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>
#include <mapnik/renderer_common/render_markers_symbolizer.hpp>

// agg
#include "agg_trans_affine.h"

// stl
#include <memory>

namespace mapnik {

struct clip_poly_tag;

using svg_attribute_type = agg::pod_bvector<svg::path_attributes>;

template <typename Detector>
struct vector_markers_dispatch : util::noncopyable
{
    vector_markers_dispatch(svg_path_ptr const& src,
                            svg_path_adapter & path,
                            svg_attribute_type const& attrs,
                            agg::trans_affine const& marker_trans,
                            symbolizer_base const& sym,
                            Detector & detector,
                            double scale_factor,
                            feature_impl const& feature,
                            attributes const& vars,
                            bool snap_to_pixels,
                            markers_renderer_context & renderer_context)
        : params_(src->bounding_box(), recenter(src) * marker_trans,
                  sym, feature, vars, scale_factor, snap_to_pixels)
        , renderer_context_(renderer_context)
        , src_(src)
        , path_(path)
        , attrs_(attrs)
        , detector_(detector)
    {}

    template <typename T>
    void add_path(T & path)
    {
        markers_placement_finder<T, Detector> placement_finder(
            params_.placement_method, path, detector_, params_.placement_params);
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
    markers_renderer_context & renderer_context_;
    svg_path_ptr const& src_;
    svg_path_adapter & path_;
    svg_attribute_type const& attrs_;
    Detector & detector_;
};

template <typename Detector>
struct raster_markers_dispatch : util::noncopyable
{
    raster_markers_dispatch(image_rgba8 const& src,
                            agg::trans_affine const& marker_trans,
                            symbolizer_base const& sym,
                            Detector & detector,
                            double scale_factor,
                            feature_impl const& feature,
                            attributes const& vars,
                            markers_renderer_context & renderer_context)
        : params_(box2d<double>(0, 0, src.width(), src.height()),
                  marker_trans, sym, feature, vars, scale_factor)
        , renderer_context_(renderer_context)
        , src_(src)
        , detector_(detector)
    {}

    template <typename T>
    void add_path(T & path)
    {
        markers_placement_finder<T, Detector> placement_finder(
            params_.placement_method, path, detector_, params_.placement_params);
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
    markers_renderer_context & renderer_context_;
    image_rgba8 const& src_;
    Detector & detector_;
};

void build_ellipse(symbolizer_base const& sym, mapnik::feature_impl & feature, attributes const& vars,
                   svg_storage_type & marker_ellipse, svg::svg_path_adapter & svg_path);

bool push_explicit_style(svg_attribute_type const& src,
                         svg_attribute_type & dst,
                         symbolizer_base const& sym,
                         feature_impl & feature,
                         attributes const& vars);

void setup_transform_scaling(agg::trans_affine & tr,
                             double svg_width,
                             double svg_height,
                             mapnik::feature_impl & feature,
                             attributes const& vars,
                             symbolizer_base const& sym);

// Apply markers to a feature with multiple geometries
template <typename Converter, typename Processor>
void apply_markers_multi(feature_impl const& feature, attributes const& vars, Converter & converter, Processor & proc, symbolizer_base const& sym)
{
    using apply_vertex_converter_type = detail::apply_vertex_converter<Converter,Processor>;
    using vertex_processor_type = geometry::vertex_processor<apply_vertex_converter_type>;

    auto const& geom = feature.get_geometry();
    geometry::geometry_types type = geometry::geometry_type(geom);

    if (type == geometry::geometry_types::Point
        || type == geometry::geometry_types::LineString
        || type == geometry::geometry_types::Polygon)
    {
        apply_vertex_converter_type apply(converter, proc);
        mapnik::util::apply_visitor(vertex_processor_type(apply), geom);
    }
    else
    {

        marker_multi_policy_enum multi_policy = get<marker_multi_policy_enum, keys::markers_multipolicy>(sym, feature, vars);
        marker_placement_enum placement = get<marker_placement_enum, keys::markers_placement_type>(sym, feature, vars);

        if (placement == MARKER_POINT_PLACEMENT &&
            multi_policy == MARKER_WHOLE_MULTI)
        {
            geometry::point<double> pt;
            // test if centroid is contained by bounding box
            if (geometry::centroid(geom, pt) && converter.disp_.args_.bbox.contains(pt.x, pt.y))
            {
                // unset any clipping since we're now dealing with a point
                converter.template unset<clip_poly_tag>();
                geometry::point_vertex_adapter<double> va(pt);
                converter.apply(va, proc);
            }
        }
        else if ((placement == MARKER_POINT_PLACEMENT || placement == MARKER_INTERIOR_PLACEMENT) &&
                 multi_policy == MARKER_LARGEST_MULTI)
        {
            // Only apply to path with largest envelope area
            // TODO: consider using true area for polygon types
            if (type == geometry::geometry_types::MultiPolygon)
            {
                geometry::multi_polygon<double> const& multi_poly = mapnik::util::get<geometry::multi_polygon<double> >(geom);
                double maxarea = 0;
                geometry::polygon<double> const* largest = 0;
                for (geometry::polygon<double> const& poly : multi_poly)
                {
                    box2d<double> bbox = geometry::envelope(poly);
                    double area = bbox.width() * bbox.height();
                    if (area > maxarea)
                    {
                        maxarea = area;
                        largest = &poly;
                    }
                }
                if (largest)
                {
                    geometry::polygon_vertex_adapter<double> va(*largest);
                    converter.apply(va, proc);
                }
            }
            else
            {
                MAPNIK_LOG_WARN(marker_symbolizer) << "TODO: if you get here -> open an issue";
            }
        }
        else
        {
            if (multi_policy != MARKER_EACH_MULTI && placement != MARKER_POINT_PLACEMENT)
            {
                MAPNIK_LOG_WARN(marker_symbolizer) << "marker_multi_policy != 'each' has no effect with marker_placement != 'point'";
            }
            apply_vertex_converter_type apply(converter, proc);
            mapnik::util::apply_visitor(vertex_processor_type(apply), geom);
        }
    }
}

}

#endif //MAPNIK_MARKER_HELPERS_HPP
