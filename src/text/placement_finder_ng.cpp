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
//mapnik
#include <mapnik/text/placement_finder_ng.hpp>
#include <mapnik/text/layout.hpp>
#include <mapnik/text_properties.hpp>
#include <mapnik/text/placements_list.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/expression_evaluator.hpp>

//boost
#include <boost/make_shared.hpp>

// agg
#include "agg_conv_clip_polyline.h"

namespace mapnik
{

placement_finder_ng::placement_finder_ng(Feature const& feature, DetectorType &detector, box2d<double> const& extent, text_placement_info_ptr placement_info, face_manager_freetype &font_manager, double scale_factor)
    : feature_(feature), detector_(detector), extent_(extent), layout_(font_manager), info_(placement_info), valid_(true), scale_factor_(scale_factor), placements_()
{
}

bool placement_finder_ng::next_position()
{
    if (!valid_)
    {
        MAPNIK_LOG_WARN(placement_finder_ng) << "next_position() called while last call already returned false!\n";
        return false;
    }
    if (!info_->next())
    {
        valid_ = false;
        return false;
    }

    info_->properties.process(layout_, feature_);
    layout_.layout(info_->properties.wrap_width, info_->properties.text_ratio);

    if (info_->properties.orientation)
    {
        angle_ = boost::apply_visitor(
            evaluate<Feature, value_type>(feature_),
            *(info_->properties.orientation)).to_double() * M_PI / 180.0;
    } else {
        angle_ = 0.0;
    }
    cosa_ = std::cos(angle_);
    sina_ = std::sin(angle_);

    init_alignment();
    return true;
}

const placements_list &placement_finder_ng::placements() const
{
    return placements_;
}


void placement_finder_ng::init_alignment()
{
    text_symbolizer_properties const& p = info_->properties;
    valign_ = p.valign;
    if (valign_ == V_AUTO)
    {
        if (p.displacement.y > 0.0)
        {
            valign_ = V_BOTTOM;
        } else if (p.displacement.y < 0.0)
        {
            valign_ = V_TOP;
        } else
        {
            valign_ = V_MIDDLE;
        }
    }

    halign_ = p.halign;
    if (halign_ == H_AUTO)
    {
        if (p.displacement.x > 0.0)
        {
            halign_ = H_RIGHT;
        } else if (p.displacement.x < 0.0)
        {
            halign_ = H_LEFT;
        } else
        {
            halign_ = H_MIDDLE;
        }
    }

    jalign_ = p.jalign;
    if (jalign_ == J_AUTO)
    {
        if (p.displacement.x > 0.0)
        {
            jalign_ = J_LEFT;
        } else if (p.displacement.x < 0.0)
        {
            jalign_ = J_RIGHT;
        } else {
            jalign_ = J_MIDDLE;
        }
    }
}


pixel_position placement_finder_ng::alignment_offset() const
{
    pixel_position result(0,0);
    // if needed, adjust for desired vertical alignment
    if (valign_ == V_TOP)
    {
        result.y = -0.5 * layout_.height();  // move center up by 1/2 the total height
    } else if (valign_ == V_BOTTOM)
    {
        result.y = 0.5 * layout_.height();  // move center down by the 1/2 the total height
    }

    // set horizontal position to middle of text
    if (halign_ == H_LEFT)
    {
        result.x = -0.5 * layout_.width();  // move center left by 1/2 the string width
    } else if (halign_ == H_RIGHT)
    {
        result.x = 0.5 * layout_.width();  // move center right by 1/2 the string width
    }
    return result;
}

// Output is centered around (0,0)
static void rotated_box2d(box2d<double> &box, double sina, double cosa, double width, double height)
{
    double new_width = width * cosa + height * sina;
    double new_height = width * sina + height * cosa;
    box.init(-new_width/2., -new_height/2., new_width/2., new_height/2.);
}

static pixel_position rotate(pixel_position pos, double sina, double cosa)
{
    double tmp_x = pos.x * cosa + pos.y * sina;
    pos.y = - pos.x * sina + pos.y * cosa;
    pos.x = tmp_x;
    return pos;
}


bool placement_finder_ng::find_point_placement(pixel_position pos)
{
    if (!layout_.size()) return true; //No text => placement always succeeds.
    glyph_positions_ptr glyphs = boost::make_shared<glyph_positions>();

    pixel_position displacement = scale_factor_ * info_->properties.displacement + alignment_offset();
    if (info_->properties.rotate_displacement) displacement = rotate(displacement, sina_, cosa_);

    glyphs->set_base_point(pos + displacement);
    box2d<double> bbox;
    rotated_box2d(bbox, sina_, cosa_, layout_.width(), layout_.height());
    bbox.re_center(glyphs->get_base_point().x, glyphs->get_base_point().y);

    if (!detector_.extent().intersects(bbox)
            ||
        (info_->properties.avoid_edges && !extent_.contains(bbox))
            ||
        (info_->properties.minimum_padding > 0 &&
         !extent_.contains(bbox + (scale_factor_ * info_->properties.minimum_padding)))
            ||
        (!info_->properties.allow_overlap &&
         !detector_.has_point_placement(bbox, info_->properties.minimum_distance * scale_factor_))
        )
    {
        return false;
    }

    detector_.insert(bbox, layout_.get_text());

    /* IMPORTANT NOTE:
       x and y are relative to the center of the text
       coordinate system:
       x: grows from left to right
       y: grows from bottom to top (opposite of normal computer graphics)
    */
    double x, y;

    // set for upper left corner of text envelope for the first line, top left of first character
    y = layout_.height() / 2.0;

    text_layout::const_iterator line_itr = layout_.begin(), line_end = layout_.end();
    for (; line_itr != line_end; line_itr++)
    {
        y -= (*line_itr)->height(); //Automatically handles first line differently
        // reset to begining of line position
        if (jalign_ == J_LEFT)
            x = -(layout_.width() / 2.0);
        else if (jalign_ == J_RIGHT)
            x = (layout_.width() / 2.0) - (*line_itr)->width();
        else
            x = -((*line_itr)->width() / 2.0);

        text_line::const_iterator glyph_itr = (*line_itr)->begin(), glyph_end = (*line_itr)->end();
        for (; glyph_itr != glyph_end; glyph_itr++)
        {
            // place the character relative to the center of the string envelope
            double dx = x * cosa_ - y * sina_;
            double dy = x * sina_ + y * cosa_;
            glyphs->push_back(*glyph_itr, pixel_position(dx, dy), angle_); //TODO: Store cosa, sina instead
            if (glyph_itr->width)
            {
                //Only advance if glyph is not part of a multiple glyph sequence
                x += glyph_itr->width + glyph_itr->format->character_spacing;
            }
        }
    }
    placements_.push_back(glyphs);
    return true;
}


template <typename T>
bool placement_finder_ng::find_point_on_line_placements(T & path)
{
    if (!layout_.size()) return true;
    vertex_cache pp(path);
    bool success = false;
    while (pp.next_subpath())
    {
        if (pp.length() == 0.0)
        {
            return find_point_placement(pp.current_position());
        }

        int num_labels = 1;
        if (info_->properties.label_spacing > 0)
            num_labels = static_cast<int> (floor(pp.length() / info_->properties.label_spacing * scale_factor_));

        if (info_->properties.force_odd_labels && num_labels % 2 == 0)
            num_labels--;
        if (num_labels <= 0)
            num_labels = 1;

        double spacing = pp.length() / num_labels;
        pp.forward(spacing/2.); // first label should be placed at half the spacing

        do
        {
            success = find_point_placement(pp.current_position()) || success;
        } while (pp.forward(spacing));
    }
    return success;
}

template <typename T>
bool placement_finder_ng::find_line_placements(T & path)
{
    if (!layout_.size()) return true;
    vertex_cache pp(path);
    bool success = false;
    while (pp.next_subpath())
    {
        if ((pp.length() < info_->properties.minimum_path_length)
                ||
            (pp.length() < layout_.width())) continue;

        int num_labels = 1;
        if (info_->properties.label_spacing > 0)
            num_labels = static_cast<int>(floor(pp.length() / (info_->properties.label_spacing * scale_factor_ + layout_.width())));

        if (info_->properties.force_odd_labels && num_labels % 2 == 0)
            num_labels--;
        if (num_labels <= 0)
            num_labels = 1;

        double spacing = pp.length() / num_labels;
        // first label should be placed at half the spacing
        pp.forward(spacing/2.-layout_.width()/2.);
        do
        {
            success = single_line_placement(pp, info_->properties.upright) || success;
        } while (pp.forward(spacing));
    }
    return success;
}


bool placement_finder_ng::single_line_placement(vertex_cache &pp, text_upright_e orientation)
{
    vertex_cache::state s = pp.save_state();
    /* IMPORTANT NOTE: See note about coordinate systems in find_point_placement()! */
    text_upright_e real_orientation = orientation;
    if (orientation == UPRIGHT_AUTO)
    {
        double angle = normalize_angle(pp.angle());
        real_orientation = (angle > 0.5*M_PI && angle < 1.5*M_PI) ? UPRIGHT_LEFT : UPRIGHT_RIGHT;
    }
    double sign = 1;
    if (real_orientation == UPRIGHT_LEFT)
    {
        sign = -1;
        pp.forward(layout_.width());
    }

    double base_offset = alignment_offset().y + info_->properties.displacement.y;
    glyph_positions_ptr glyphs = boost::make_shared<glyph_positions>();

    double offset = base_offset + layout_.height();
    unsigned upside_down_glyph_count = 0;

    text_layout::const_iterator line_itr = layout_.begin(), line_end = layout_.end();
    for (; line_itr != line_end; line_itr++)
    {
        double char_height = (*line_itr)->max_char_height();
        offset -= (*line_itr)->height();
        pp.set_offset(offset);

        text_line::const_iterator glyph_itr = (*line_itr)->begin(), glyph_end = (*line_itr)->end();
        for (; glyph_itr != glyph_end; glyph_itr++)
        {
            double angle = normalize_angle(pp.angle(sign * glyph_itr->width));
            double sina = sin(angle);
            double cosa = cos(angle);
            pixel_position pos = pp.current_position();
            //Center the text on the line
            pos.y = -pos.y - char_height/2.0*cosa;
            pos.x =  pos.x + char_height/2.0*sina;

            if (angle > M_PI/2. && angle < 1.5*M_PI)
                upside_down_glyph_count++;

            glyphs->push_back(*glyph_itr, pos, angle); //TODO: Store cosa, sina instead
            if (glyph_itr->width)
            {
                //Only advance if glyph is not part of a multiple glyph sequence
                pp.move(sign * (glyph_itr->width + glyph_itr->format->character_spacing));
            }
        }
    }
    pp.restore_state(s);
    if (orientation == UPRIGHT_AUTO && (upside_down_glyph_count > layout_.size()/2))
    {
        //Try again with oposite orienation
        return single_line_placement(pp, real_orientation == UPRIGHT_RIGHT ? UPRIGHT_LEFT : UPRIGHT_RIGHT);
    }
    placements_.push_back(glyphs);
    return true;
}

double placement_finder_ng::normalize_angle(double angle)
{
    while (angle >= 2*M_PI)
        angle -= 2*M_PI;
    while (angle < 0)
        angle += 2*M_PI;
    return angle;
}


/*********************************************************************************************/


glyph_positions::glyph_positions()
    : base_point_(), const_angle_(true)
{

}

glyph_positions::const_iterator glyph_positions::begin() const
{
    return data_.begin();
}

glyph_positions::const_iterator glyph_positions::end() const
{
    return data_.end();
}

void glyph_positions::push_back(const glyph_info &glyph, pixel_position offset, double angle)
{
    if (data_.empty())
    {
        angle_ = angle;
    } else
    {
        if (angle != angle_) const_angle_ = false;
    }
    data_.push_back(glyph_position(glyph, offset, angle));
}


bool glyph_positions::is_constant_angle() const
{
    return const_angle_;
}

double glyph_positions::get_angle() const
{
    return angle_;
}

pixel_position const& glyph_positions::get_base_point() const
{
    return base_point_;
}

void glyph_positions::set_base_point(pixel_position base_point)
{
    base_point_ = base_point;
}


/*************************************************************************************/
typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
typedef coord_transform<CoordTransform,clipped_geometry_type> ClippedPathType;
typedef coord_transform<CoordTransform,geometry_type> PathType;
template bool placement_finder_ng::find_point_on_line_placements<ClippedPathType>(ClippedPathType &);
template bool placement_finder_ng::find_line_placements<ClippedPathType>(ClippedPathType &);
template bool placement_finder_ng::find_point_on_line_placements<PathType>(PathType &);
template bool placement_finder_ng::find_line_placements<PathType>(PathType &);


}// ns mapnik
