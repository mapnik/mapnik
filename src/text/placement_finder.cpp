/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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
#include <mapnik/ctrans.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/text/placement_finder_impl.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/placements_list.hpp>
#include <mapnik/text/vertex_cache.hpp>
#include <mapnik/text/tolerance_iterator.hpp>

// agg
#include "agg_conv_clip_polyline.h"

// stl
#include <vector>

namespace mapnik
{

placement_finder::placement_finder(feature_impl const& feature,
                                   attributes const& attr,
                                   DetectorType &detector,
                                   box2d<double> const& extent,
                                   text_placement_info & placement_info,
                                   face_manager_freetype & font_manager,
                                   double scale_factor)
    : feature_(feature),
      attr_(attr),
      detector_(detector),
      extent_(extent),
      info_(placement_info),
      scale_factor_(scale_factor),
      font_manager_(font_manager),
      placements_(),
      has_marker_(false),
      marker_(),
      marker_box_() {}

bool placement_finder::next_position()
{
    if (info_.next())
    {
        text_layout_ptr layout = std::make_shared<text_layout>(font_manager_, scale_factor_, info_.properties.layout_defaults);
        layout->evaluate_properties(feature_, attr_);
        move_dx_ = layout->displacement().x;
        info_.properties.process(*layout, feature_, attr_);
        layouts_.clear(); // FIXME !!!!
        layouts_.add(layout);
        layouts_.layout();
        horizontal_alignment_ = layout->horizontal_alignment();
        return true;
    }
    MAPNIK_LOG_WARN(placement_finder) << "next_position() called while last call already returned false!\n";
    return false;
}

text_upright_e placement_finder::simplify_upright(text_upright_e upright, double angle) const
{
    if (upright == UPRIGHT_AUTO)
    {
        return (std::fabs(normalize_angle(angle)) > 0.5*M_PI) ? UPRIGHT_LEFT : UPRIGHT_RIGHT;
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
    glyph_positions_ptr glyphs = std::make_shared<glyph_positions>();
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

        if (layout.num_lines()) bboxes.push_back(std::move(bbox));

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
                glyphs->push_back(glyph, (pixel_position(x, y).rotate(orientation)) + layout_offset, orientation);
                if (glyph.advance())
                {
                    //Only advance if glyph is not part of a multiple glyph sequence
                    x += glyph.advance() + glyph.format->character_spacing * scale_factor_;
                }
            }
        }
    }

    // add_marker first checks for collision and then updates the detector.
    if (has_marker_ && !add_marker(glyphs, pos)) return false;

    for (box2d<double> const& bbox : bboxes)
    {
        detector_.insert(bbox, layouts_.text());
    }
    placements_.push_back(glyphs);

    return true;
}

bool placement_finder::single_line_placement(vertex_cache &pp, text_upright_e orientation)
{
    //
    // IMPORTANT NOTE: See note about coordinate systems in find_point_placement()!
    //

    vertex_cache::scoped_state begin(pp);
    text_upright_e real_orientation = simplify_upright(orientation, pp.angle());

    glyph_positions_ptr glyphs = std::make_shared<glyph_positions>();
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
        double offset = layout_displacement.y + 0.5 * sign * layout.height();

        for (auto const& line : layout)
        {
            // Only subtract half the line height here and half at the end because text is automatically
            // centered on the line
            offset -= sign * line.height()/2;
            vertex_cache & off_pp = pp.get_offseted(offset, sign*layout.width());
            vertex_cache::scoped_state off_state(off_pp); // TODO: Remove this when a clean implementation in vertex_cache::get_offseted is done

            if (!off_pp.move(sign * layout.jalign_offset(line.width()) - align_offset.x)) return false;

            double last_cluster_angle = 999;
            int current_cluster = -1;
            pixel_position cluster_offset;
            double angle;
            rotation rot;
            double last_glyph_spacing = 0.;

            for (auto const& glyph : line)
            {
                if (current_cluster != static_cast<int>(glyph.char_index))
                {
                    if (!off_pp.move(sign * (layout.cluster_width(current_cluster) + last_glyph_spacing)))
                    {
                        return false;
                    }
                    current_cluster = glyph.char_index;
                    last_glyph_spacing = glyph.format->character_spacing * scale_factor_;
                    // Only calculate new angle at the start of each cluster!
                    angle = normalize_angle(off_pp.angle(sign * layout.cluster_width(current_cluster)));
                    rot.init(angle);
                    if ((info_.properties.max_char_angle_delta > 0) && (last_cluster_angle != 999) &&
                            std::fabs(normalize_angle(angle-last_cluster_angle)) > info_.properties.max_char_angle_delta)
                    {
                        return false;
                    }
                    cluster_offset.clear();
                    last_cluster_angle = angle;
                }

                if (std::abs(angle) > M_PI/2) ++upside_down_glyph_count;

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
                glyphs->push_back(glyph, pos, rot);
            }
            // See comment above
            offset -= sign * line.height()/2;
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
        // upright==left_only or right_only and more than 50% of characters upside down => no placement
        else if (orientation == UPRIGHT_LEFT_ONLY || orientation == UPRIGHT_RIGHT_ONLY)
        {
            return false;
        }
    }

    for (box2d<double> const& box : bboxes)
    {
        detector_.insert(box, layouts_.text());
    }
    placements_.push_back(glyphs);

    return true;
}

void placement_finder::path_move_dx(vertex_cache & pp, double dx)
{
    vertex_cache::state state = pp.save_state();
    if (!pp.move(dx)) pp.restore_state(state);
}

double placement_finder::normalize_angle(double angle)
{
    while (angle >= M_PI)
    {
        angle -= 2.0 * M_PI;
    }
    while (angle < -M_PI)
    {
        angle += 2.0 * M_PI;
    }
    return angle;
}

double placement_finder::get_spacing(double path_length, double layout_width) const
{
    int num_labels = 1;
    if (info_.properties.label_spacing > 0)
    {
        num_labels = static_cast<int>(floor(
            path_length / (info_.properties.label_spacing * scale_factor_ + layout_width)));
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
        margin = info_.properties.margin * scale_factor_;
        repeat_distance = (info_.properties.repeat_distance != 0 ? info_.properties.repeat_distance : info_.properties.minimum_distance) * scale_factor_;
    }
    else
    {
        margin = (info_.properties.margin != 0 ? info_.properties.margin : info_.properties.minimum_distance) * scale_factor_;
        repeat_distance = info_.properties.repeat_distance * scale_factor_;
    }
    return !detector_.extent().intersects(box)
               ||
           (info_.properties.avoid_edges && !extent_.contains(box))
               ||
           (info_.properties.minimum_padding > 0 &&
            !extent_.contains(box + (scale_factor_ * info_.properties.minimum_padding)))
               ||
           (!info_.properties.allow_overlap &&
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


bool placement_finder::add_marker(glyph_positions_ptr glyphs, pixel_position const& pos) const
{
    pixel_position real_pos = (marker_unlocked_ ? pos : glyphs->get_base_point()) + marker_displacement_;
    box2d<double> bbox = marker_box_;
    bbox.move(real_pos.x, real_pos.y);
    glyphs->set_marker(marker_, real_pos);
    if (collision(bbox, layouts_.text(), false)) return false;
    detector_.insert(bbox);
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


glyph_positions::glyph_positions()
    : data_(),
      base_point_(),
      marker_(),
      marker_pos_(),
      bbox_() {}

glyph_positions::const_iterator glyph_positions::begin() const
{
    return data_.begin();
}

glyph_positions::const_iterator glyph_positions::end() const
{
    return data_.end();
}

void glyph_positions::push_back(glyph_info const& glyph, pixel_position const offset, rotation const& rot)
{
    data_.push_back(glyph_position(glyph, offset, rot));
}

void glyph_positions::reserve(unsigned count)
{
    data_.reserve(count);
}

pixel_position const& glyph_positions::get_base_point() const
{
    return base_point_;
}

void glyph_positions::set_base_point(pixel_position const base_point)
{
    base_point_ = base_point;
}

void glyph_positions::set_marker(marker_info_ptr marker, pixel_position const& marker_pos)
{
    marker_ = marker;
    marker_pos_ = marker_pos;
}

marker_info_ptr glyph_positions::marker() const
{
    return marker_;
}

pixel_position const& glyph_positions::marker_pos() const
{
    return marker_pos_;
}

}// ns mapnik
