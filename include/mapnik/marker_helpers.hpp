/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#include <mapnik/color.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geom_util.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/svg/svg_path_attributes.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/marker.hpp> // for svg_storage_type
#include <mapnik/markers_placement.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/label_collision_detector.hpp>

// agg
#include "agg_trans_affine.h"

// boost
#include <boost/optional.hpp>

// stl
#include <memory>
#include <type_traits> // remove_reference
#include <cmath>

namespace mapnik {

struct clip_poly_tag;

using svg_attribute_type = agg::pod_bvector<svg::path_attributes>;

template <typename Detector>
struct vector_markers_dispatch : mapnik::noncopyable
{
    vector_markers_dispatch(svg_path_ptr const& src,
                            agg::trans_affine const& marker_trans,
                            symbolizer_base const& sym,
                            Detector & detector,
                            double scale_factor,
                            feature_impl & feature,
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
        marker_placement_enum placement_method = get<marker_placement_enum>(sym_, keys::markers_placement_type, feature_, vars_, MARKER_POINT_PLACEMENT);
        bool ignore_placement = get<bool>(sym_, keys::ignore_placement, feature_, vars_, false);
        bool allow_overlap = get<bool>(sym_, keys::allow_overlap, feature_, vars_, false);
        bool avoid_edges = get<bool>(sym_, keys::avoid_edges, feature_, vars_, false);
        double opacity = get<double>(sym_,keys::opacity, feature_, vars_, 1.0);
        double spacing = get<double>(sym_, keys::spacing, feature_, vars_, 100.0);
        double max_error = get<double>(sym_, keys::max_error, feature_, vars_, 0.2);
        box2d<double> const& bbox = src_->bounding_box();
        coord2d center = bbox.center();
        agg::trans_affine_translation recenter(-center.x, -center.y);
        agg::trans_affine tr = recenter * marker_trans_;
        markers_placement_params params { bbox, tr, spacing * scale_factor_, max_error, allow_overlap, avoid_edges };
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
    feature_impl & feature_;
    attributes const& vars_;
    double scale_factor_;
};

template <typename Detector>
struct raster_markers_dispatch : mapnik::noncopyable
{
    raster_markers_dispatch(image_data_32 & src,
                            agg::trans_affine const& marker_trans,
                            symbolizer_base const& sym,
                            Detector & detector,
                            double scale_factor,
                            feature_impl & feature,
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
        marker_placement_enum placement_method = get<marker_placement_enum>(sym_, keys::markers_placement_type, feature_, vars_, MARKER_POINT_PLACEMENT);
        bool allow_overlap = get<bool>(sym_, keys::allow_overlap, feature_, vars_, false);
        bool avoid_edges = get<bool>(sym_, keys::avoid_edges, feature_, vars_, false);
        double opacity = get<double>(sym_, keys::opacity, feature_, vars_, 1.0);
        bool ignore_placement = get<bool>(sym_, keys::ignore_placement, feature_, vars_, false);
        double spacing = get<double>(sym_, keys::spacing, feature_, vars_, 100.0);
        double max_error = get<double>(sym_, keys::max_error, feature_, vars_, 0.2);
        box2d<double> bbox(0,0, src_.width(), src_.height());
        markers_placement_params params { bbox, marker_trans_, spacing * scale_factor_, max_error, allow_overlap, avoid_edges };
        markers_placement_finder<T, label_collision_detector4> placement_finder(
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
    image_data_32 & src_;
    agg::trans_affine const& marker_trans_;
    symbolizer_base const& sym_;
    Detector & detector_;
    feature_impl & feature_;
    attributes const& vars_;
    double scale_factor_;
};

void build_ellipse(symbolizer_base const& sym, mapnik::feature_impl & feature, attributes const& vars, svg_storage_type & marker_ellipse, svg::svg_path_adapter & svg_path);

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
template <typename Converter>
void apply_markers_multi(feature_impl const& feature, attributes const& vars, Converter & converter, symbolizer_base const& sym)
{
    std::size_t geom_count = feature.paths().size();
    if (geom_count == 1)
    {
        converter.apply(const_cast<geometry_type&>(feature.paths()[0]));
    }
    else if (geom_count > 1)
    {
        marker_multi_policy_enum multi_policy = get<marker_multi_policy_enum>(sym, keys::markers_multipolicy, feature, vars, MARKER_EACH_MULTI);
        marker_placement_enum placement = get<marker_placement_enum>(sym, keys::markers_placement_type, feature, vars, MARKER_POINT_PLACEMENT);

        if (placement == MARKER_POINT_PLACEMENT &&
            multi_policy == MARKER_WHOLE_MULTI)
        {
            double x, y;
            if (label::centroid_geoms(feature.paths().begin(), feature.paths().end(), x, y))
            {
                geometry_type pt(geometry_type::types::Point);
                pt.move_to(x, y);
                // unset any clipping since we're now dealing with a point
                converter.template unset<clip_poly_tag>();
                converter.apply(pt);
            }
        }
        else if ((placement == MARKER_POINT_PLACEMENT || placement == MARKER_INTERIOR_PLACEMENT) &&
                 multi_policy == MARKER_LARGEST_MULTI)
        {
            // Only apply to path with largest envelope area
            // TODO: consider using true area for polygon types
            double maxarea = 0;
            geometry_type const* largest = 0;
            for (geometry_type const& geom : feature.paths())
            {
                const box2d<double>& env = geom.envelope();
                double area = env.width() * env.height();
                if (area > maxarea)
                {
                    maxarea = area;
                    largest = &geom;
                }
            }
            if (largest)
            {
                converter.apply(const_cast<geometry_type&>(*largest));
            }
        }
        else
        {
            if (multi_policy != MARKER_EACH_MULTI && placement != MARKER_POINT_PLACEMENT)
            {
                MAPNIK_LOG_WARN(marker_symbolizer) << "marker_multi_policy != 'each' has no effect with marker_placement != 'point'";
            }
            for (geometry_type const& path : feature.paths())
            {
                converter.apply(const_cast<geometry_type&>(path));
            }
        }
    }
}

}

#endif //MAPNIK_MARKER_HELPERS_HPP
