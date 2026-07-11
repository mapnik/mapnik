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

#if defined(HAVE_CAIRO)

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/image_compositing.hpp>
#include <mapnik/cairo/cairo_context.hpp>
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/renderer_common.hpp>
#include <mapnik/renderer_common/arc_text_placement.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/util/math.hpp>

// stl
#include <algorithm>
#include <cmath>
#include <memory>

namespace mapnik {

namespace detail {

// Renders a circular arc / sector ("pie wedge") at every point vertex of a
// geometry, mimicking the arcs OpenSeaMap uses to show the visibility sectors
// of naval navigation lights.
//
// Angle convention: start_angle / end_angle are compass bearings in degrees,
// measured clockwise from north (up). The sector is swept clockwise from
// start_angle to end_angle. This matches how light sectors are charted; change
// the angle->point mapping below if a different convention is needed.
struct render_arc_symbolizer
{
    template<typename Adapter>
    void operator()(Adapter const& va) const
    {
        double x, y, z = 0;
        unsigned cmd = SEG_END;
        va.rewind(0);
        while ((cmd = va.vertex(&x, &y)) != mapnik::SEG_END)
        {
            if (cmd == SEG_CLOSE)
                continue;
            prj_trans_.backward(x, y, z);
            common_.t_.forward(&x, &y);
            render_one(x, y);
        }
    }

    // Sweep angles in radians, clockwise from north, with wrap-around handled
    // (e.g. 350 -> 10 degrees). Equal angles mean a full circle, as charted
    // for all-round lights. Returns {a0, a1} with a1 > a0.
    std::pair<double, double> sweep() const
    {
        double a0 = util::radians(start_angle_);
        double a1 = util::radians(end_angle_);
        if (a1 <= a0)
            a1 += util::tau;
        return {a0, a1};
    }

    // Point on the arc at angle a (bearing clockwise from north, cairo pixel
    // space with y pointing down).
    void arc_point(double cx, double cy, double a, double& px, double& py) const
    {
        px = cx + radius_ * std::sin(a);
        py = cy - radius_ * std::cos(a);
    }

    // Adds the curved part of the arc to the current path via cairo_arc. cairo
    // uses standard math angles (from +x); our bearings are clockwise from north,
    // which maps to (bearing - pi/2). cairo_arc sweeps in the increasing-angle
    // direction, which is clockwise in cairo's y-down space, matching our sweep.
    void append_arc(double cx, double cy) const
    {
        auto [a0, a1] = sweep();
        double const half_pi = util::tau / 4.0;
        context_.arc(cx, cy, radius_, a0 - half_pi, a1 - half_pi);
    }

    // Closed pie wedge (center -> arc start, arc, arc end -> center), used for fill.
    void build_wedge(double cx, double cy) const
    {
        context_.move_to(cx, cy);
        append_arc(cx, cy); // cairo connects the current point to the arc start
        context_.close_path();
    }

    // The two radius spokes from the center to the arc endpoints.
    void build_radius_lines(double cx, double cy) const
    {
        auto [a0, a1] = sweep();
        double px, py;
        context_.move_to(cx, cy);
        arc_point(cx, cy, a0, px, py);
        context_.line_to(px, py);
        context_.move_to(cx, cy);
        arc_point(cx, cy, a1, px, py);
        context_.line_to(px, py);
    }

    void render_one(double cx, double cy) const
    {
        if (radius_ <= 0.0)
            return;

        // Fill is always the closed pie wedge.
        if (has_fill_)
        {
            build_wedge(cx, cy);
            context_.set_color(fill_, fill_opacity_);
            context_.fill();
        }

        // Draw the radius spokes
        if (has_radius_stroke_)
        {
            build_radius_lines(cx, cy);
            context_.set_line_width(radius_stroke_width_);
            context_.set_color(radius_stroke_, radius_stroke_opacity_);
            context_.stroke();
        }

        // Draw the curved arc line
        if (has_arc_stroke_)
        {
            append_arc(cx, cy);
            context_.set_line_width(arc_stroke_width_);
            context_.set_color(arc_stroke_, arc_stroke_opacity_);
            context_.stroke();
        }
    }

    cairo_context& context_;
    renderer_common const& common_;
    proj_transform const& prj_trans_;
    double radius_;
    double start_angle_;
    double end_angle_;

    color fill_;
    double fill_opacity_;
    bool has_fill_;

    color stroke_;
    double stroke_width_;
    double stroke_opacity_;
    bool has_stroke_;

    color arc_stroke_;
    double arc_stroke_width_;
    double arc_stroke_opacity_;
    bool has_arc_stroke_;

    color radius_stroke_;
    double radius_stroke_width_;
    double radius_stroke_opacity_;
    bool has_radius_stroke_;
};

} // namespace detail

template<typename T>
void cairo_renderer<T>::process(arc_symbolizer const& sym,
                                mapnik::feature_impl& feature,
                                proj_transform const& prj_trans)
{
    double const radius = get<double>(sym, keys::radius, feature, common_.vars_, 0.0) * common_.scale_factor_;
    double const start_angle = get<double>(sym, keys::start_angle, feature, common_.vars_, 0.0);
    double const end_angle = get<double>(sym, keys::end_angle, feature, common_.vars_, 360.0);

    // fill attributes
    bool const has_fill = has_key(sym, keys::fill);
    color const fill = get<mapnik::color>(sym, keys::fill, feature, common_.vars_, mapnik::color(128, 128, 128));
    double const fill_opacity = get<double>(sym, keys::fill_opacity, feature, common_.vars_, 1.0);

    // default stroke attributes
    bool const has_stroke = has_key(sym, keys::stroke);
    color const stroke = get<mapnik::color>(sym, keys::stroke, feature, common_.vars_, mapnik::color(0, 0, 0));
    double const stroke_width_raw = get<double>(sym, keys::stroke_width, feature, common_.vars_, 1.0);
    double const stroke_width = stroke_width_raw * common_.scale_factor_;
    double const stroke_opacity = get<double>(sym, keys::stroke_opacity, feature, common_.vars_, 1.0);

    // arc stroke attributes -- using default stroke attributes as fallback
    bool const has_arc_stroke = has_key(sym, keys::arc_stroke) || has_stroke;
    color const arc_stroke = has_key(sym, keys::arc_stroke)
                           ? get<mapnik::color>(sym, keys::arc_stroke, feature, common_.vars_, mapnik::color(0, 0, 0))
                           : stroke;
    double const arc_stroke_width_raw = has_key(sym, keys::arc_stroke_width)
                           ? get<double>(sym, keys::arc_stroke_width, feature, common_.vars_, 1.0)
                           : stroke_width_raw;
    double const arc_stroke_width = arc_stroke_width_raw * common_.scale_factor_;
    double const arc_stroke_opacity = has_key(sym, keys::arc_stroke_opacity)
                                    ? get<double>(sym, keys::arc_stroke_opacity, feature, common_.vars_, 1.0)
                                    : stroke_opacity;

    // radius stroke attributes -- using default stroke attributes as fallback
    bool const has_radius_stroke = has_key(sym, keys::radius_stroke) || has_stroke;
    color const radius_stroke = has_key(sym, keys::radius_stroke)
                              ? get<mapnik::color>(sym, keys::radius_stroke, feature, common_.vars_, mapnik::color(0, 0, 0))
                              : stroke;
    double const radius_stroke_width_raw = has_key(sym, keys::radius_stroke_width)
                           ? get<double>(sym, keys::radius_stroke_width, feature, common_.vars_, 1.0)
                           : stroke_width_raw;
    double const radius_stroke_width = radius_stroke_width_raw * common_.scale_factor_;
    double const radius_stroke_opacity = has_key(sym, keys::radius_stroke_opacity)
                                       ? get<double>(sym, keys::radius_stroke_opacity, feature, common_.vars_, 1.0)
                                       : stroke_opacity;

    cairo_save_restore guard(context_);

    detail::render_arc_symbolizer apply{context_,
                                        common_,
                                        prj_trans,
                                        radius,
                                        start_angle,
                                        end_angle,
                                        fill,
                                        fill_opacity,
                                        has_fill,
                                        stroke,
                                        stroke_width,
                                        stroke_opacity,
                                        has_stroke,
                                        arc_stroke,
                                        arc_stroke_width,
                                        arc_stroke_opacity,
                                        has_arc_stroke,
                                        radius_stroke,
                                        radius_stroke_width,
                                        radius_stroke_opacity,
                                        has_radius_stroke};
    mapnik::util::apply_visitor(geometry::vertex_processor<detail::render_arc_symbolizer>(apply),
                                feature.get_geometry());
}

template void cairo_renderer<cairo_ptr>::process(arc_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik

#endif // HAVE_CAIRO
