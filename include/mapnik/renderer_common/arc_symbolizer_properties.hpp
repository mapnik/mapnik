/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_RENDERER_COMMON_ARC_SYMBOLIZER_PROPERTIES_HPP
#define MAPNIK_RENDERER_COMMON_ARC_SYMBOLIZER_PROPERTIES_HPP

#include <mapnik/color.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/util/math.hpp>

#include <cmath>
#include <utility>

namespace mapnik {

// Style attributes of an arc_symbolizer, evaluated once per feature and
// shared by the agg, cairo and svg backend implementations so the fallback
// rules stay identical across renderers.
//
// The arc-stroke-* and radius-stroke-* attributes fall back to the general
// stroke-* attributes, so a plain stroke styles both the curved arc line and
// the radius spokes while either group can be overridden individually. The
// has_* flags tell whether the respective part should be drawn at all.
//
// Widths and the radius are already multiplied by the renderer's scale factor.
struct arc_symbolizer_properties
{
    arc_symbolizer_properties(arc_symbolizer const& sym,
                              feature_impl const& feature,
                              attributes const& vars,
                              double scale_factor)
    {
        radius = get<double>(sym, keys::radius, feature, vars, 0.0) * scale_factor;
        start_angle = normalize_bearing(get<double>(sym, keys::start_angle, feature, vars, 0.0));
        end_angle = normalize_bearing(get<double>(sym, keys::end_angle, feature, vars, 360.0));

        // fill attributes
        has_fill = has_key(sym, keys::fill);
        fill = get<mapnik::color>(sym, keys::fill, feature, vars, mapnik::color(128, 128, 128));
        fill_opacity = get<double>(sym, keys::fill_opacity, feature, vars, 1.0);

        // default stroke attributes
        has_stroke = has_key(sym, keys::stroke);
        stroke = get<mapnik::color>(sym, keys::stroke, feature, vars, mapnik::color(0, 0, 0));
        double const stroke_width_raw = get<double>(sym, keys::stroke_width, feature, vars, 1.0);
        stroke_width = stroke_width_raw * scale_factor;
        stroke_opacity = get<double>(sym, keys::stroke_opacity, feature, vars, 1.0);
        auto stroke_dash_opt = get_optional<dash_array>(sym, keys::stroke_dasharray, feature, vars);
        if (stroke_dash_opt)
            stroke_dash = *stroke_dash_opt;
        stroke_dash_offset = get<double>(sym, keys::stroke_dashoffset, feature, vars, 0.0);

        // arc stroke attributes -- using default stroke attributes as fallback
        has_arc_stroke = has_key(sym, keys::arc_stroke) || has_stroke;
        arc_stroke = has_key(sym, keys::arc_stroke)
                       ? get<mapnik::color>(sym, keys::arc_stroke, feature, vars, mapnik::color(0, 0, 0))
                       : stroke;
        arc_stroke_width = has_key(sym, keys::arc_stroke_width)
                             ? get<double>(sym, keys::arc_stroke_width, feature, vars, 1.0) * scale_factor
                             : stroke_width;
        arc_stroke_opacity = has_key(sym, keys::arc_stroke_opacity)
                               ? get<double>(sym, keys::arc_stroke_opacity, feature, vars, 1.0)
                               : stroke_opacity;
        auto arc_dash_opt = has_key(sym, keys::arc_stroke_dasharray)
                              ? get_optional<dash_array>(sym, keys::arc_stroke_dasharray, feature, vars)
                              : stroke_dash_opt;
        if (arc_dash_opt)
            arc_dash = *arc_dash_opt;
        arc_dash_offset = has_key(sym, keys::arc_stroke_dashoffset)
                            ? get<double>(sym, keys::arc_stroke_dashoffset, feature, vars, 0.0)
                            : stroke_dash_offset;

        // radius spoke stroke attributes -- using default stroke attributes as fallback
        has_radius_stroke = has_key(sym, keys::radius_stroke) || has_stroke;
        radius_stroke = has_key(sym, keys::radius_stroke)
                          ? get<mapnik::color>(sym, keys::radius_stroke, feature, vars, mapnik::color(0, 0, 0))
                          : stroke;
        radius_stroke_width = has_key(sym, keys::radius_stroke_width)
                                ? get<double>(sym, keys::radius_stroke_width, feature, vars, 1.0) * scale_factor
                                : stroke_width;
        radius_stroke_opacity = has_key(sym, keys::radius_stroke_opacity)
                                  ? get<double>(sym, keys::radius_stroke_opacity, feature, vars, 1.0)
                                  : stroke_opacity;
        auto radius_dash_opt = has_key(sym, keys::radius_stroke_dasharray)
                                 ? get_optional<dash_array>(sym, keys::radius_stroke_dasharray, feature, vars)
                                 : stroke_dash_opt;
        if (radius_dash_opt)
            radius_dash = *radius_dash_opt;
        radius_dash_offset = has_key(sym, keys::radius_stroke_dashoffset)
                               ? get<double>(sym, keys::radius_stroke_dashoffset, feature, vars, 0.0)
                               : stroke_dash_offset;

        // label attributes -- the text itself and its formatting live in the
        // text_placements_ property, only the radial gap is a plain attribute
        text_offset = get<double>(sym, keys::text_offset, feature, vars, 2.0) * scale_factor;
    }

    // Reduce a bearing in degrees to [0, 360), so that e.g. 360 and 0, or 400
    // and 40, describe the same direction. Non-finite bearings become NaN.
    static double normalize_bearing(double angle)
    {
        angle = std::fmod(angle, 360.0);
        if (angle < 0.0)
            angle += 360.0; // may round up to 360 for tiny negative angles
        return angle >= 360.0 ? 0.0 : angle;
    }

    // Whether there is anything to draw at all.
    bool drawable() const
    {
        return radius > 0.0 && std::isfinite(radius) && std::isfinite(start_angle) && std::isfinite(end_angle);
    }

    // Sweep angles in radians, clockwise from north, with wrap-around handled
    // (e.g. 350 -> 10 degrees). Equal angles mean a full circle, as charted
    // for all-round lights. Returns {a0, a1} with a0 < a1 <= a0 + tau.
    std::pair<double, double> sweep() const
    {
        double a0 = util::radians(start_angle);
        double a1 = util::radians(end_angle);
        if (a1 <= a0)
            a1 += util::tau;
        return {a0, a1};
    }

    double radius;
    double start_angle;
    double end_angle;

    bool has_fill;
    color fill;
    double fill_opacity;

    bool has_stroke;
    color stroke;
    double stroke_width;
    double stroke_opacity;
    dash_array stroke_dash;
    double stroke_dash_offset;

    bool has_arc_stroke;
    color arc_stroke;
    double arc_stroke_width;
    double arc_stroke_opacity;
    dash_array arc_dash;
    double arc_dash_offset;

    bool has_radius_stroke;
    color radius_stroke;
    double radius_stroke_width;
    double radius_stroke_opacity;
    dash_array radius_dash;
    double radius_dash_offset;

    double text_offset;
};

} // namespace mapnik

#endif // MAPNIK_RENDERER_COMMON_ARC_SYMBOLIZER_PROPERTIES_HPP
