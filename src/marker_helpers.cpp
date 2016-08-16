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

// mapnik
#include <mapnik/marker_helpers.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/label_collision_detector.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_ellipse.h"
#include "agg_color_rgba.h"
#pragma GCC diagnostic pop

namespace mapnik {

void build_ellipse(symbolizer_base const& sym, mapnik::feature_impl & feature, attributes const& vars, svg_storage_type & marker_ellipse, svg::svg_path_adapter & svg_path)
{
    double width = 0.0;
    double height = 0.0;
    double half_stroke_width = 0.0;
    if (has_key(sym,keys::width) && has_key(sym,keys::height))
    {
        width = get<double>(sym, keys::width, feature, vars, 0.0);
        height = get<double>(sym, keys::height, feature, vars, 0.0);
    }
    else if (has_key(sym,keys::width))
    {
        width = height = get<double>(sym, keys::width, feature, vars, 0.0);
    }
    else if (has_key(sym,keys::height))
    {
        width = height = get<double>(sym, keys::height, feature, vars, 0.0);
    }
    if (has_key(sym,keys::stroke_width))
    {
        half_stroke_width = get<double>(sym, keys::stroke_width, feature, vars, 0.0) / 2.0;
    }
    svg::svg_converter_type styled_svg(svg_path, marker_ellipse.attributes());
    styled_svg.push_attr();
    styled_svg.begin_path();
    agg::ellipse c(0, 0, width/2.0, height/2.0);
    styled_svg.storage().concat_path(c);
    styled_svg.end_path();
    styled_svg.pop_attr();
    double lox,loy,hix,hiy;
    styled_svg.bounding_rect(&lox, &loy, &hix, &hiy);
    lox -= half_stroke_width;
    loy -= half_stroke_width;
    hix += half_stroke_width;
    hiy += half_stroke_width;
    styled_svg.set_dimensions(width,height);
    marker_ellipse.set_dimensions(width,height);
    marker_ellipse.set_bounding_box(lox,loy,hix,hiy);
}

bool push_explicit_style(svg_attribute_type const& src,
                         svg_attribute_type & dst,
                         symbolizer_base const& sym,
                         feature_impl & feature,
                         attributes const& vars)
{
    auto fill_color = get_optional<color>(sym, keys::fill, feature, vars);
    auto fill_opacity = get_optional<double>(sym, keys::fill_opacity, feature, vars);
    auto stroke_color = get_optional<color>(sym, keys::stroke, feature, vars);
    auto stroke_width = get_optional<double>(sym, keys::stroke_width, feature, vars);
    auto stroke_opacity = get_optional<double>(sym, keys::stroke_opacity, feature, vars);
    if (fill_color ||
        fill_opacity ||
        stroke_color ||
        stroke_width ||
        stroke_opacity)
    {
        bool success = false;
        for(unsigned i = 0; i < src.size(); ++i)
        {
            dst.push_back(src[i]);
            mapnik::svg::path_attributes & attr = dst.last();
            if (!attr.visibility_flag)
                continue;
            success = true;

            if (!attr.stroke_none)
            {
                if (stroke_width)
                {
                    attr.stroke_width = *stroke_width;
                    attr.stroke_flag = true;
                }
                if (stroke_color)
                {
                    color const& s_color = *stroke_color;
                    attr.stroke_color = agg::rgba(s_color.red()/255.0,
                                                  s_color.green()/255.0,
                                                  s_color.blue()/255.0,
                                                  s_color.alpha()/255.0);
                    attr.stroke_flag = true;
                }
                if (stroke_opacity)
                {
                    attr.stroke_opacity = *stroke_opacity;
                    attr.stroke_flag = true;
                }
            }
            if (!attr.fill_none)
            {
                if (fill_color)
                {
                    color const& f_color = *fill_color;
                    attr.fill_color = agg::rgba(f_color.red()/255.0,
                                                f_color.green()/255.0,
                                                f_color.blue()/255.0,
                                                f_color.alpha()/255.0);
                    attr.fill_flag = true;
                }
                if (fill_opacity)
                {
                    attr.fill_opacity = *fill_opacity;
                    attr.fill_flag = true;
                }
            }
        }
        return success;
    }
    return false;
}

void setup_transform_scaling(agg::trans_affine & tr,
                             double svg_width,
                             double svg_height,
                             mapnik::feature_impl & feature,
                             attributes const& vars,
                             symbolizer_base const& sym)
{
    double width = get<double>(sym, keys::width, feature, vars, 0.0);
    double height = get<double>(sym, keys::height, feature, vars, 0.0);
    if (width > 0 && height > 0)
    {
        double sx = width/svg_width;
        double sy = height/svg_height;
        tr *= agg::trans_affine_scaling(sx,sy);
    }
    else if (width > 0)
    {
        double sx = width/svg_width;
        tr *= agg::trans_affine_scaling(sx);
    }
    else if (height > 0)
    {
        double sy = height/svg_height;
        tr *= agg::trans_affine_scaling(sy);
    }
}

template <typename Processor>
void apply_markers_single(vertex_converter_type & converter, Processor & proc,
                          geometry::geometry<double> const& geom, geometry::geometry_types type)
{
    if (type == geometry::geometry_types::Point)
    {
        geometry::point_vertex_adapter<double> va(geom.get<geometry::point<double>>());
        converter.apply(va, proc);
    }
    else if (type == geometry::geometry_types::LineString)
    {
        geometry::line_string_vertex_adapter<double> va(geom.get<geometry::line_string<double>>());
        converter.apply(va, proc);
    }
    else if (type == geometry::geometry_types::Polygon)
    {
        geometry::polygon_vertex_adapter<double> va(geom.get<geometry::polygon<double>>());
        converter.apply(va, proc);
    }
    else if (type == geometry::geometry_types::MultiPoint)
    {
        for (auto const& pt : geom.get<geometry::multi_point<double>>())
        {
            geometry::point_vertex_adapter<double> va(pt);
            converter.apply(va, proc);
        }
    }
    else if (type == geometry::geometry_types::MultiLineString)
    {
        for (auto const& line : geom.get<geometry::multi_line_string<double>>())
        {
            geometry::line_string_vertex_adapter<double> va(line);
            converter.apply(va, proc);
        }
    }
    else if (type == geometry::geometry_types::MultiPolygon)
    {
        for (auto const& poly : geom.get<geometry::multi_polygon<double>>())
        {
            geometry::polygon_vertex_adapter<double> va(poly);
            converter.apply(va, proc);
        }
    }
}

template <typename Processor>
void apply_markers_multi(feature_impl const& feature, attributes const& vars,
                         vertex_converter_type & converter, Processor & proc, symbolizer_base const& sym)
{
    auto const& geom = feature.get_geometry();
    geometry::geometry_types type = geometry::geometry_type(geom);

    if (type == geometry::geometry_types::Point
        ||
        type == geometry::geometry_types::LineString
        ||
        type == geometry::geometry_types::Polygon)
    {
        apply_markers_single(converter, proc, geom, type);
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
            if (type == geometry::geometry_types::GeometryCollection)
            {
                for (auto const& g : geom.get<geometry::geometry_collection<double>>())
                {
                    apply_markers_single(converter, proc, g, geometry::geometry_type(g));
                }
            }
            else
            {
                apply_markers_single(converter, proc, geom, type);
            }
        }
    }
}
template void apply_markers_multi<vector_dispatch_type>(feature_impl const& feature, attributes const& vars,
                                                        vertex_converter_type & converter, vector_dispatch_type & proc,
                                                        symbolizer_base const& sym);

template void apply_markers_multi<raster_dispatch_type>(feature_impl const& feature, attributes const& vars,
                                                        vertex_converter_type & converter, raster_dispatch_type & proc,
                                                        symbolizer_base const& sym);

} // end namespace mapnik
