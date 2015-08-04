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
//mapnik
#include <mapnik/debug.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/view_transform.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/text/placement_finder_impl.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/glyph_info.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/util/math.hpp>

// stl
#include <vector>

namespace mapnik
{

placement_finder::placement_finder(feature_impl const& feature,
                                   attributes const& attr,
                                   DetectorType &detector,
                                   box2d<double> const& extent,
                                   text_placement_info const& placement_info,
                                   face_manager_freetype & font_manager,
                                   double scale_factor)
    : feature_(feature),
      attr_(attr),
      detector_(detector),
      extent_(extent),
      info_(placement_info),
      text_props_(evaluate_text_properties(info_.properties,feature_,attr_)),
      scale_factor_(scale_factor),
      font_manager_(font_manager),
      placements_(),
      has_marker_(false),
      marker_(),
      marker_box_(),
      marker_unlocked_(false),
      marker_displacement_(),
      move_dx_(0.0),
      horizontal_alignment_(H_LEFT) {}

bool placement_finder::next_position()
{
    if (info_.next())
    {
        // parent layout, has top-level ownership of a new evaluated_format_properties_ptr (TODO is this good enough to stay in scope???)
        // but does not take ownership of the text_symbolizer_properties (info_.properties)
        text_layout_ptr layout = std::make_shared<text_layout>(font_manager_,
                                                               feature_,
                                                               attr_,
                                                               scale_factor_,
                                                               info_.properties,
                                                               info_.properties.layout_defaults,
                                                               info_.properties.format_tree());
        // TODO: why is this call needed?
        // https://github.com/mapnik/mapnik/issues/2525
        text_props_ = evaluate_text_properties(info_.properties,feature_,attr_);
        // Note: this clear call is needed when multiple placements are tried
        // like with placement-type="simple|list"
        if (!layouts_.empty()) layouts_.clear();
        // Note: multiple layouts_ may result from this add() call
        layouts_.add(layout);
        layouts_.layout();
        // cache a few values for use elsewhere in placement finder
        move_dx_ = layout->displacement().x;
        horizontal_alignment_ = layout->horizontal_alignment();
        return true;
    }
    return false;
}

text_upright_e placement_finder::simplify_upright(text_upright_e upright, double angle) const
{
    if (upright == UPRIGHT_AUTO)
    {
        return (std::fabs(util::normalize_angle(angle)) > 0.5*M_PI) ? UPRIGHT_LEFT : UPRIGHT_RIGHT;
    }
    if (upright == UPRIGHT_AUTO_DOWN)
    {
        return (std::fabs(util::normalize_angle(angle)) < 0.5*M_PI) ? UPRIGHT_LEFT : UPRIGHT_RIGHT;
    }
    if (upright == UPRIGHT_LEFT_ONLY)
    {
        return UPRIGHT_LEFT;
    }
    if (upright == UPRIGHT_RIGHT_ONLY)
    {
        return  UPRIGHT_RIGHT;
    }
    return upright;
}

bool placement_finder::find_point_placement(pixel_position const& pos)
{
    glyph_positions_ptr glyphs = std::make_unique<glyph_positions>();
    std::vector<box2d<double> > bboxes;

    glyphs->reserve(layouts_.glyphs_count());
    bboxes.reserve(layouts_.size());

    bool base_point_set = false;
    for (auto const& layout_ptr : layouts_)
    {
        text_layout const& layout = *layout_ptr;
        rotation const& orientation = layout.orientation();

        // Find text origin.
        pixel_position layout_center = pos + layout.displacement();

        if (!base_point_set)
        {
            glyphs->set_base_point(layout_center);
            base_point_set = true;
        }

        box2d<double> bbox = layout.bounds();
        bbox.re_center(layout_center.x, layout_center.y);

        /* For point placements it is faster to just check the bounding box. */
        if (collision(bbox, layouts_.text(), false)) return false;

        if (layout.glyphs_count()) bboxes.push_back(std::move(bbox));

        pixel_position layout_offset = layout_center - glyphs->get_base_point();
        layout_offset.y = -layout_offset.y;

        // IMPORTANT NOTE:
        //   x and y are relative to the center of the text
        //   coordinate system:
        //   x: grows from left to right
        //   y: grows from bottom to top (opposite of normal computer graphics)

        double x, y;

        // set for upper left corner of text envelope for the first line, top left of first character
        y = layout.height() / 2.0;

        for ( auto const& line : layout)
        {
            y -= line.height(); //Automatically handles first line differently
            x = layout.jalign_offset(line.width());

            for (auto const& glyph : line)
            {
                // place the character relative to the center of the string envelope
                glyphs->emplace_back(glyph, (pixel_position(x, y).rotate(orientation)) + layout_offset, orientation);
                if (glyph.advance())
                {
                    //Only advance if glyph is not part of a multiple glyph sequence
                    x += glyph.advance() + glyph.format->character_spacing * scale_factor_;
                }
            }
        }
    }

    // add_marker first checks for collision and then updates the detector.
    if (has_marker_ && !add_marker(glyphs, pos, bboxes)) return false;

    box2d<double> label_box;
    bool first = true;
    for (box2d<double> const& box : bboxes)
    {
        if (first)
        {
            label_box = box;
            first = false;
        }
        else
        {
            label_box.expand_to_include(box);
        }
        detector_.insert(box, layouts_.text());
    }
    // do not render text off the canvas
    if (extent_.intersects(label_box))
    {
        placements_.push_back(std::move(glyphs));
    }

    return true;
}

bool placement_finder::single_line_placement(vertex_cache &pp, text_upright_e orientation)
{
    //
    // IMPORTANT NOTE: See note about coordinate systems in find_point_placement()!
    //

    vertex_cache::scoped_state begin(pp);
    text_upright_e real_orientation = simplify_upright(orientation, pp.angle());

    glyph_positions_ptr glyphs = std::make_unique<glyph_positions>();
    std::vector<box2d<double> > bboxes;
    glyphs->reserve(layouts_.glyphs_count());
    bboxes.reserve(layouts_.glyphs_count());

    unsigned upside_down_glyph_count = 0;

    for (auto const& layout_ptr : layouts_)
    {
        text_layout const& layout = *layout_ptr;
        pixel_position align_offset = layout.alignment_offset();
        pixel_position const& layout_displacement = layout.displacement();
        double sign = (real_orientation == UPRIGHT_LEFT) ? -1 : 1;
        //double offset = 0 - (layout_displacement.y + 0.5 * sign * layout.height());
        double offset = layout_displacement.y - 0.5 * sign * layout.height();
        double adjust_character_spacing = .0;
        double layout_width = layout.width();
        bool adjust = layout.horizontal_alignment() == H_ADJUST;

        if (adjust)
        {
            text_layout::const_iterator longest_line = layout.longest_line();
            if (longest_line != layout.end())
            {
                adjust_character_spacing = (pp.length() - longest_line->glyphs_width()) / longest_line->space_count();
                layout_width = longest_line->glyphs_width() + longest_line->space_count() * adjust_character_spacing;
            }
        }

        for (auto const& line : layout)
        {
            // Only subtract half the line height here and half at the end because text is automatically
            // centered on the line
            offset += sign * line.height()/2;
            vertex_cache & off_pp = pp.get_offseted(offset, sign * layout_width);
            vertex_cache::scoped_state off_state(off_pp); // TODO: Remove this when a clean implementation in vertex_cache::get_offseted is done
            double line_width = adjust ? (line.glyphs_width() + line.space_count() * adjust_character_spacing) : line.width();

            if (!off_pp.move(sign * layout.jalign_offset(line_width) - align_offset.x)) return false;

            double last_cluster_angle = 999;
            int current_cluster = -1;
            pixel_position cluster_offset;
            double angle = 0;
            rotation rot;
            double last_glyph_spacing = 0.0;
            for (auto const& glyph : line)
            {
                if (current_cluster != static_cast<int>(glyph.char_index))
                {
                    if (adjust)
                    {
                        if (!off_pp.move(sign * (layout.cluster_width(current_cluster) + last_glyph_spacing)))
                        {
                            return false;
                        }
                        last_glyph_spacing = adjust_character_spacing;
                    }
                    else
                    {
                        if (!off_pp.move_to_distance(sign * (layout.cluster_width(current_cluster) + last_glyph_spacing)))
                        {
                            return false;
                        }
                        last_glyph_spacing = glyph.format->character_spacing * scale_factor_;
                    }
                    current_cluster = glyph.char_index;
                    // Only calculate new angle at the start of each cluster!
                    // Y axis is inverted.
                    // See note about coordinate systems in placement_finder::find_point_placement().
                    angle = -util::normalize_angle(off_pp.angle(sign * layout.cluster_width(current_cluster)));
                    rot.init(angle);
                    if ((text_props_->max_char_angle_delta > 0) && (last_cluster_angle != 999) &&
                        std::fabs(util::normalize_angle(angle - last_cluster_angle)) > text_props_->max_char_angle_delta)
                    {
                        return false;
                    }
                    cluster_offset.clear();
                    last_cluster_angle = angle;
                }

                if (std::abs(angle) > M_PI/2)
                {
                    ++upside_down_glyph_count;
                }

                pixel_position pos = off_pp.current_position() + cluster_offset;
                // Center the text on the line
                double char_height = line.max_char_height();
                pos.y = -pos.y - char_height/2.0*rot.cos;
                pos.x =  pos.x + char_height/2.0*rot.sin;

                cluster_offset.x += rot.cos * glyph.advance();
                cluster_offset.y -= rot.sin * glyph.advance();

                box2d<double> bbox = get_bbox(layout, glyph, pos, rot);
                if (collision(bbox, layouts_.text(), true)) return false;
                bboxes.push_back(std::move(bbox));
                glyphs->emplace_back(glyph, pos, rot);
            }
            // See comment above
            offset += sign * line.height()/2;
        }
    }

    if (upside_down_glyph_count > static_cast<unsigned>(layouts_.text().length() / 2))
    {
        if (orientation == UPRIGHT_AUTO)
        {
            // Try again with opposite orientation
            begin.restore();
            return single_line_placement(pp, real_orientation == UPRIGHT_RIGHT ? UPRIGHT_LEFT : UPRIGHT_RIGHT);
        }
        // upright==left-only or right-only and more than 50% of characters upside down => no placement
        else if (orientation == UPRIGHT_LEFT_ONLY || orientation == UPRIGHT_RIGHT_ONLY)
        {
            return false;
        }
    }
    else if (orientation == UPRIGHT_AUTO_DOWN)
    {
        // Try again with opposite orientation
        begin.restore();
        return single_line_placement(pp, real_orientation == UPRIGHT_RIGHT ? UPRIGHT_LEFT : UPRIGHT_RIGHT);
    }

    box2d<double> label_box;
    bool first = true;
    for (box2d<double> const& box : bboxes)
    {
        if (first)
        {
            label_box = box;
            first = false;
        }
        else
        {
            label_box.expand_to_include(box);
        }
        detector_.insert(box, layouts_.text());
    }
    // do not render text off the canvas
    if (extent_.intersects(label_box))
    {
        placements_.push_back(std::move(glyphs));
    }

    return true;
}

void placement_finder::path_move_dx(vertex_cache & pp, double dx)
{
    vertex_cache::state state = pp.save_state();
    if (!pp.move(dx)) pp.restore_state(state);
}

double placement_finder::get_spacing(double path_length, double layout_width) const
{
    int num_labels = 1;
    if (horizontal_alignment_ != H_ADJUST && text_props_->label_spacing > 0)
    {
        num_labels = static_cast<int>(std::floor(
                                          path_length / (text_props_->label_spacing * scale_factor_ + layout_width)));
    }
    if (num_labels <= 0)
    {
        num_labels = 1;
    }
    return path_length / num_labels;
}

bool placement_finder::collision(const box2d<double> &box, const value_unicode_string &repeat_key, bool line_placement) const
{
    double margin, repeat_distance;
    if (line_placement)
    {
        margin = text_props_->margin * scale_factor_;
        repeat_distance = (text_props_->repeat_distance != 0 ? text_props_->repeat_distance : text_props_->minimum_distance) * scale_factor_;
    }
    else
    {
        margin = (text_props_->margin != 0 ? text_props_->margin : text_props_->minimum_distance) * scale_factor_;
        repeat_distance = text_props_->repeat_distance * scale_factor_;
    }
    return (text_props_->avoid_edges && !extent_.contains(box))
        ||
        (text_props_->minimum_padding > 0 &&
         !extent_.contains(box + (scale_factor_ * text_props_->minimum_padding)))
        ||
        (!text_props_->allow_overlap &&
         ((repeat_key.length() == 0 && !detector_.has_placement(box, margin))
          ||
          (repeat_key.length() > 0 && !detector_.has_placement(box, margin, repeat_key, repeat_distance))));
}

void placement_finder::set_marker(marker_info_ptr m, box2d<double> box, bool marker_unlocked, pixel_position const& marker_displacement)
{
    marker_ = m;
    marker_box_ = box * scale_factor_;
    marker_displacement_ = marker_displacement * scale_factor_;
    marker_unlocked_ = marker_unlocked;
    has_marker_ = true;
}


bool placement_finder::add_marker(glyph_positions_ptr & glyphs, pixel_position const& pos, std::vector<box2d<double>> & bboxes) const
{
    pixel_position real_pos = (marker_unlocked_ ? pos : glyphs->get_base_point()) + marker_displacement_;
    box2d<double> bbox = marker_box_;
    bbox.move(real_pos.x, real_pos.y);
    if (collision(bbox, layouts_.text(), false)) return false;
    detector_.insert(bbox);
    bboxes.push_back(std::move(bbox));
    glyphs->set_marker(marker_, real_pos);
    return true;
}

box2d<double> placement_finder::get_bbox(text_layout const& layout, glyph_info const& glyph, pixel_position const& pos, rotation const& rot)
{
    /*

      (0/ymax)           (width/ymax)
      ***************
      *             *
      (0/0)*             *
      *             *
      ***************
      (0/ymin)          (width/ymin)
      Add glyph offset in y direction, but not in x direction (as we use the full cluster width anyways)!
    */
    double width = layout.cluster_width(glyph.char_index);
    if (glyph.advance() <= 0) width = -width;
    pixel_position tmp, tmp2;
    tmp.set(0, glyph.ymax());
    tmp = tmp.rotate(rot);
    tmp2.set(width, glyph.ymax());
    tmp2 = tmp2.rotate(rot);
    box2d<double> bbox(tmp.x,  -tmp.y,
                       tmp2.x, -tmp2.y);
    tmp.set(width, glyph.ymin());
    tmp = tmp.rotate(rot);
    bbox.expand_to_include(tmp.x, -tmp.y);
    tmp.set(0, glyph.ymin());
    tmp = tmp.rotate(rot);
    bbox.expand_to_include(tmp.x, -tmp.y);
    pixel_position pos2 = pos + pixel_position(0, glyph.offset.y).rotate(rot);
    bbox.move(pos2.x , -pos2.y);
    return bbox;
}

}// ns mapnik
