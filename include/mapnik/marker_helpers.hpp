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

#ifndef MAPNIK_MARKER_HELPERS_HPP
#define MAPNIK_MARKER_HELPERS_HPP

#include <mapnik/feature.hpp>
#include <mapnik/color.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/geometry_centroid.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/marker.hpp> // for svg_storage_type
#include <mapnik/markers_placement.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/renderer_common/apply_vertex_converter.hpp>

// agg
#include "agg_trans_affine.h"

// stl
#include <memory>

namespace mapnik {

struct clip_poly_tag;

namespace svg { struct path_attributes; }

using svg_attribute_type = agg::pod_bvector<svg::path_attributes>;

template <typename Detector>
struct vector_markers_dispatch : util::noncopyable
{
    vector_markers_dispatch(svg_path_ptr const& src,
                            agg::trans_affine const& marker_trans,
                            symbolizer_base const& sym,
                            Detector & detector,
                            double scale_factor,
                            feature_impl const& feature,
                            attributes const& vars)
        : src_(src),
          marker_trans_(marker_trans),
          sym_(sym),
          detector_(detector),
          feature_(feature),
          vars_(vars),
          scale_factor_(scale_factor)
    {}

    virtual ~vector_markers_dispatch() {}

    template <typename T>
    void add_path(T & path)
    {
        marker_placement_enum placement_method = get<marker_placement_enum, keys::markers_placement_type>(sym_, feature_, vars_);
        value_bool ignore_placement = get<value_bool, keys::ignore_placement>(sym_, feature_, vars_);
        value_bool allow_overlap = get<value_bool, keys::allow_overlap>(sym_, feature_, vars_);
        value_bool avoid_edges = get<value_bool, keys::avoid_edges>(sym_, feature_, vars_);
        value_double opacity = get<value_double,keys::opacity>(sym_, feature_, vars_);
        value_double spacing = get<value_double, keys::spacing>(sym_, feature_, vars_);
        value_double max_error = get<value_double, keys::max_error>(sym_, feature_, vars_);
        coord2d center = src_->bounding_box().center();
        agg::trans_affine_translation recenter(-center.x, -center.y);
        agg::trans_affine tr = recenter * marker_trans_;
        direction_enum direction = get<direction_enum, keys::direction>(sym_, feature_, vars_);
        markers_placement_params params { src_->bounding_box(), tr, spacing * scale_factor_, max_error, allow_overlap, avoid_edges, direction };
        markers_placement_finder<T, Detector> placement_finder(
            placement_method, path, detector_, params);
        double x, y, angle = .0;
        while (placement_finder.get_point(x, y, angle, ignore_placement))
        {
            agg::trans_affine matrix = tr;
            matrix.rotate(angle);
            matrix.translate(x, y);
            render_marker(matrix, opacity);
        }
    }

    virtual void render_marker(agg::trans_affine const& marker_tr, double opacity) = 0;

protected:
    svg_path_ptr const& src_;
    agg::trans_affine const& marker_trans_;
    symbolizer_base const& sym_;
    Detector & detector_;
    feature_impl const& feature_;
    attributes const& vars_;
    double scale_factor_;
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
                            attributes const& vars)
    : src_(src),
        marker_trans_(marker_trans),
        sym_(sym),
        detector_(detector),
        feature_(feature),
        vars_(vars),
        scale_factor_(scale_factor)
    {}

    virtual ~raster_markers_dispatch() {}

    template <typename T>
    void add_path(T & path)
    {
        marker_placement_enum placement_method = get<marker_placement_enum, keys::markers_placement_type>(sym_, feature_, vars_);
        value_bool allow_overlap = get<value_bool, keys::allow_overlap>(sym_, feature_, vars_);
        value_bool avoid_edges = get<value_bool, keys::avoid_edges>(sym_, feature_, vars_);
        value_double opacity = get<value_double, keys::opacity>(sym_, feature_, vars_);
        value_bool ignore_placement = get<value_bool, keys::ignore_placement>(sym_, feature_, vars_);
        value_double spacing = get<value_double, keys::spacing>(sym_, feature_, vars_);
        value_double max_error = get<value_double, keys::max_error>(sym_, feature_, vars_);
        box2d<double> bbox(0,0, src_.width(),src_.height());
        direction_enum direction = get<direction_enum, keys::direction>(sym_, feature_, vars_);
        markers_placement_params params { bbox, marker_trans_, spacing * scale_factor_, max_error, allow_overlap, avoid_edges, direction };
        markers_placement_finder<T, Detector> placement_finder(
            placement_method, path, detector_, params);
        double x, y, angle = .0;
        while (placement_finder.get_point(x, y, angle, ignore_placement))
        {
            agg::trans_affine matrix = marker_trans_;
            matrix.rotate(angle);
            matrix.translate(x, y);
            render_marker(matrix, opacity);
        }
    }

    virtual void render_marker(agg::trans_affine const& marker_tr, double opacity) = 0;

protected:
    image_rgba8 const& src_;
    agg::trans_affine const& marker_trans_;
    symbolizer_base const& sym_;
    Detector & detector_;
    feature_impl const& feature_;
    attributes const& vars_;
    double scale_factor_;
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
    using vertex_converter_type = Converter;
    using apply_vertex_converter_type = detail::apply_vertex_converter<vertex_converter_type,Processor>;
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
                    geometry::polygon_vertex_adapter<double> va(poly);
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
