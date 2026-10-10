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

#include <mapnik/renderer_common/arc_text_placement.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/symbolizer_keys.hpp>
#include <mapnik/text/placement_finder.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/text_line.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/util/math.hpp>

#include <cmath>
#include <utility>
#include <vector>

namespace mapnik {

namespace {

// convert from polar to cartesian within the glyphs coordinate system
pixel_position polar(double bearing, double r)
{
    return pixel_position(r * std::sin(bearing), r * std::cos(bearing));
}

// Advance pen position by one glyph
double pen_advance(glyph_info const& glyph, double scale_factor)
{
    double advance = glyph.advance();
    if (advance)
        advance += glyph.format->character_spacing * scale_factor;
    return advance;
}

} // namespace

arc_text_layout::arc_text_layout(arc_symbolizer const& sym,
                                 feature_impl const& feature,
                                 attributes const& vars,
                                 face_manager_freetype& font_manager,
                                 double scale_factor)
{
    text_placements_ptr const placements = mapnik::get<text_placements_ptr>(sym, keys::text_placements_);
    if (!placements)
        return;

    info_ = placements->get_placement_info(scale_factor, feature, vars);
    if (!info_ || !info_->next())
    {
        info_.reset();
        return;
    }

    auto layout = std::make_unique<text_layout>(font_manager,
                                                feature,
                                                vars,
                                                scale_factor,
                                                info_->properties,
                                                info_->properties.layout_defaults,
                                                info_->properties.format_tree());
    layout->layout();
    if (layout->glyphs_count() == 0)
    {
        info_.reset();
        return;
    }
    layout_ = std::move(layout);
    allow_overlap_ = evaluate_text_properties(info_->properties, feature, vars)->allow_overlap;
}

arc_text_layout::~arc_text_layout() = default;

glyph_positions_ptr place_arc_text(arc_text_layout const& label,
                                   double cx,
                                   double cy,
                                   double radius,
                                   double a0,
                                   double a1,
                                   double text_offset,
                                   double scale_factor,
                                   label_collision_detector4& detector)
{
    text_layout const& layout = label.get();
    if (layout.num_lines() == 0)
        return glyph_positions_ptr();

    text_line const& line = *layout.begin();
    double const width = line.width();
    if (width <= 0.0)
        return glyph_positions_ptr();

    double const char_height = line.max_char_height();
    double const mid = (a0 + a1) * 0.5;

    auto glyphs = std::make_unique<glyph_positions>();
    glyphs->reserve(layout.glyphs_count());
    glyphs->set_base_point(pixel_position(cx, cy));
    std::vector<box2d<double>> bboxes;
    bboxes.reserve(layout.glyphs_count());

    // On the lower half of the circle text bent along the arc would come out
    // upside down, so it is turned around there and reads counter-clockwise.
    // Its glyphs then grow towards the arc instead of away from it, which the
    // baseline has to compensate for by another line height.
    bool const flip = std::cos(mid) < 0.0;
    double const baseline_radius = radius + text_offset + (flip ? char_height : 0.0);

    if (width <= (a1 - a0) * baseline_radius)
    {
        // If the text will fit the available arc space we will place the
        // text along the arc, one individually placed and rotated glyph
        // at a time

        double const dir = flip ? -1.0 : 1.0;

        // calculate the start bearing of the text
        double bearing = mid - dir * 0.5 * width / baseline_radius;

        // now process text glyphs one at a time, rotating them along
        // the arc in the proper direction and orientation
        for (auto const& glyph : line)
        {
            rotation const rot(flip ? util::pi - bearing : -bearing);
            pixel_position const pos = polar(bearing, baseline_radius);
            bboxes.push_back(glyph_bbox(layout, glyph, pos, rot));
            glyphs->emplace_back(glyph, pos, rot);
            bearing += dir * pen_advance(glyph, scale_factor) / baseline_radius;
        }
    }
    else
    {
        // text is too long to fit on the arc, so place it radially along the
        // ray that bisects the sector

        // on the left half of the circle text running outwards would read
        // right to left, so it is turned around and runs towards the arc
        bool const inward = std::sin(mid) < 0.0;

        // angle at which the text shall point
        double const angle = util::pi / 2.0 - mid + (inward ? util::pi : 0.0);
        rotation const rot(angle);

        // Center the glyphs on the ray by shifting half a glyph height
        // against the glyphs' "up" direction (-sin, cos).
        pixel_position const centering(0.5 * char_height * rot.sin, -0.5 * char_height * rot.cos);

        // starting position depends on text direction
        double const dir = inward ? -1.0 : 1.0;
        double r = radius + text_offset + (inward ? width : 0.0);

        // now place the glyphs one at a time, along the determined direction
        for (auto const& glyph : line)
        {
            pixel_position const pos = polar(mid, r) + centering;
            bboxes.push_back(glyph_bbox(layout, glyph, pos, rot));
            glyphs->emplace_back(glyph, pos, rot);
            r += dir * pen_advance(glyph, scale_factor);
        }
    }

    // glyph boxes are relative to the base point, the detector works in screen space
    for (auto& box : bboxes)
    {
        box.move(cx, cy);
        if (!label.allow_overlap() && !detector.has_placement(box))
            return glyph_positions_ptr();
    }
    for (auto const& box : bboxes)
    {
        detector.insert(box, layout.text());
    }
    return glyphs;
}

} // namespace mapnik
