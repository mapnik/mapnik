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
#include <mapnik/renderer_common/arc_symbolizer_properties.hpp>
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

    // Point on the arc at angle a (bearing clockwise from north, cairo pixel
    // space with y pointing down).
    void arc_point(double cx, double cy, double a, double& px, double& py) const
    {
        px = cx + props_.radius * std::sin(a);
        py = cy - props_.radius * std::cos(a);
    }

    // Adds the curved part of the arc to the current path via cairo_arc. cairo
    // uses standard math angles (from +x); our bearings are clockwise from north,
    // which maps to (bearing - pi/2). cairo_arc sweeps in the increasing-angle
    // direction, which is clockwise in cairo's y-down space, matching our sweep.
    void append_arc(double cx, double cy) const
    {
        auto [a0, a1] = props_.sweep();
        double const half_pi = util::tau / 4.0;
        context_.arc(cx, cy, props_.radius, a0 - half_pi, a1 - half_pi);
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
        auto [a0, a1] = props_.sweep();
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
        if (props_.radius <= 0.0)
            return;

        // Fill is always the closed pie wedge.
        if (props_.has_fill)
        {
            build_wedge(cx, cy);
            context_.set_color(props_.fill, props_.fill_opacity);
            context_.fill();
        }

        // Draw the radius spokes
        if (props_.has_radius_stroke)
        {
            cairo_save_restore guard(context_);
            if (!props_.radius_dash.empty())
                context_.set_dash(props_.radius_dash, common_.scale_factor_, props_.radius_dash_offset);
            build_radius_lines(cx, cy);
            context_.set_line_width(props_.radius_stroke_width);
            context_.set_color(props_.radius_stroke, props_.radius_stroke_opacity);
            context_.stroke();
        }

        // Draw the curved arc line
        if (props_.has_arc_stroke)
        {
            cairo_save_restore guard(context_);
            if (!props_.arc_dash.empty())
                context_.set_dash(props_.arc_dash, common_.scale_factor_, props_.arc_dash_offset);
            append_arc(cx, cy);
            context_.set_line_width(props_.arc_stroke_width);
            context_.set_color(props_.arc_stroke, props_.arc_stroke_opacity);
            context_.stroke();
        }

        // Label bent along the outside of the arc, or running radially outward
        // when the arc is too short for it. Colours and halo come from the
        // glyph formats.
        if (layout_)
        {
            // add_text switches the cairo operator, so keep that scoped
            cairo_save_restore guard(context_);
            auto [a0, a1] = props_.sweep();
            glyph_positions_ptr glyphs =
              place_arc_text(*layout_, cx, cy, props_.radius, a0, a1, props_.text_offset, common_.scale_factor_);
            if (glyphs)
                context_.add_text(*glyphs, face_manager_, comp_op_, src_over, common_.scale_factor_);
        }
    }

    cairo_context& context_;
    renderer_common const& common_;
    proj_transform const& prj_trans_;
    arc_symbolizer_properties const& props_;
    text_layout const* layout_;
    cairo_face_manager& face_manager_;
    composite_mode_e comp_op_;
};

} // namespace detail

template<typename T>
void cairo_renderer<T>::process(arc_symbolizer const& sym,
                                mapnik::feature_impl& feature,
                                proj_transform const& prj_trans)
{
    arc_symbolizer_properties const props(sym, feature, common_.vars_, common_.scale_factor_);
    composite_mode_e const comp_op = get<composite_mode_e>(sym, keys::comp_op, feature, common_.vars_, src_over);

    // Shape the optional label once here; it is placed per arc centre below.
    arc_text_layout const label(sym, feature, common_.vars_, common_.font_manager_, common_.scale_factor_);

    cairo_save_restore guard(context_);
    context_.set_operator(comp_op);

    detail::render_arc_symbolizer
      apply{context_, common_, prj_trans, props, label ? &label.get() : nullptr, face_manager_, comp_op};
    mapnik::util::apply_visitor(geometry::vertex_processor<detail::render_arc_symbolizer>(apply),
                                feature.get_geometry());
}

template void cairo_renderer<cairo_ptr>::process(arc_symbolizer const&, mapnik::feature_impl&, proj_transform const&);

} // namespace mapnik

#endif // HAVE_CAIRO
