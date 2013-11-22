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
#include <mapnik/text/placement_finder.hpp>
#include <mapnik/text/layout.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/placements_list.hpp>
#include <mapnik/text/vertex_cache.hpp>

// agg
#include "agg_conv_clip_polyline.h"

// stl
#include <vector>

namespace mapnik
{

class tolerance_iterator
{
public:
    tolerance_iterator(double label_position_tolerance, double spacing)
        : tolerance_(label_position_tolerance > 0 ?
                        label_position_tolerance : spacing/2.0),
          tolerance_delta_(std::max(1.0, tolerance_/100.0)),
          value_(0),
          initialized_(false),
          values_tried_(0)
    {
    }

    ~tolerance_iterator()
    {
        //std::cout << "values tried:" << values_tried_ << "\n";
    }

    double get() const
    {
        return -value_;
    }

    bool next()
    {
        ++values_tried_;
        if (values_tried_ > 255)
        {
            /* This point should not be reached during normal operation. But I can think of
             * cases where very bad spacing and or tolerance values are choosen and the
             * placement finder tries an excessive number of placements.
             * 255 is an arbitrarily chosen limit.
             */
            MAPNIK_LOG_WARN(placement_finder) << "Tried a huge number of placements. Please check "
                                                 "'label-position-tolerance' and 'spacing' parameters "
                                                 "of your TextSymbolizers.\n";
            return false;
        }
        if (!initialized_)
        {
            initialized_ = true;
            return true; //Always return value 0 as the first value.
        }
        if (value_ == 0)
        {
            value_ = tolerance_delta_;
            return true;
        }
        value_ = -value_;
        if (value_ > 0)
        {
            value_ += tolerance_delta_;
        }
        if (value_ > tolerance_)
        {
            return false;
        }
        return true;
    }
private:
    double tolerance_;
    double tolerance_delta_;
    double value_;
    bool initialized_;
    unsigned values_tried_;
};


// Output is centered around (0,0)
static void rotated_box2d(box2d<double> & box, rotation const& rot, double width, double height)
{
    double new_width = width * rot.cos + height * rot.sin;
    double new_height = width * rot.sin + height * rot.cos;
    box.init(-new_width/2., -new_height/2., new_width/2., new_height/2.);
}

pixel_position pixel_position::rotate(rotation const& rot) const
{
    return pixel_position(x * rot.cos - y * rot.sin, x * rot.sin + y * rot.cos);
}

placement_finder::placement_finder(feature_impl const& feature,
                                   DetectorType &detector,
                                   box2d<double> const& extent,
                                   text_placement_info_ptr placement_info,
                                   face_manager_freetype & font_manager,
                                   double scale_factor)
    : feature_(feature),
      detector_(detector),
      extent_(extent),
      layout_(font_manager, scale_factor),
      info_(placement_info),
      valid_(true),
      scale_factor_(scale_factor),
      placements_(),
      has_marker_(false),
      marker_(),
      marker_box_()
{
}

bool placement_finder::next_position()
{
    if (!valid_)
    {
        MAPNIK_LOG_WARN(placement_finder) << "next_position() called while last call already returned false!\n";
        return false;
    }
    if (!info_->next())
    {
        valid_ = false;
        return false;
    }

    info_->properties.process(layout_, feature_);
    layout_.layout(info_->properties.wrap_width * scale_factor_, info_->properties.text_ratio, info_->properties.wrap_before);

    if (info_->properties.orientation)
    {
        // https://github.com/mapnik/mapnik/issues/1352
        mapnik::evaluate<feature_impl, value_type> evaluator(feature_);
        orientation_.init(
            boost::apply_visitor(
            evaluator,
            *(info_->properties.orientation)).to_double() * M_PI / 180.0);
    }
    else
    {
        orientation_.reset();
    }
    init_alignment();
    return true;
}

void placement_finder::init_alignment()
{
    text_symbolizer_properties const& p = info_->properties;
    valign_ = p.valign;
    if (valign_ == V_AUTO)
    {
        if (p.displacement.y > 0.0)
        {
            valign_ = V_BOTTOM;
        }
        else if (p.displacement.y < 0.0)
        {
            valign_ = V_TOP;
        }
        else
        {
            valign_ = V_MIDDLE;
        }
    }

    halign_point_ = p.halign;
    halign_line_ = p.halign;
    if (halign_point_ == H_AUTO)
    {
        if (p.displacement.x > 0.0)
        {
            halign_point_ = H_RIGHT;
            halign_line_ = H_LEFT;
        }
        else if (p.displacement.x < 0.0)
        {
            halign_point_ = H_LEFT;
            halign_line_= H_RIGHT;
        }
        else
        {
            halign_point_ = H_MIDDLE;
            halign_line_ = H_MIDDLE;
        }
    }

    jalign_ = p.jalign;
    if (jalign_ == J_AUTO)
    {
        if (p.displacement.x > 0.0)
        {
            jalign_ = J_LEFT;
        }
        else if (p.displacement.x < 0.0)
        {
            jalign_ = J_RIGHT;
        }
        else
        {
            jalign_ = J_MIDDLE;
        }
    }
}


pixel_position placement_finder::alignment_offset() const //TODO
{
    pixel_position result(0,0);
    // if needed, adjust for desired vertical alignment
    if (valign_ == V_TOP)
    {
        result.y = -0.5 * layout_.height();  // move center up by 1/2 the total height
    }
    else if (valign_ == V_BOTTOM)
    {
        result.y = 0.5 * layout_.height();  // move center down by the 1/2 the total height
    }

    // set horizontal position to middle of text
    if (halign_point_ == H_LEFT)
    {
        result.x = -0.5 * layout_.width();  // move center left by 1/2 the string width
    }
    else if (halign_point_ == H_RIGHT)
    {
        result.x = 0.5 * layout_.width();  // move center right by 1/2 the string width
    }
    return result;
}

double placement_finder::jalign_offset(double line_width) const //TODO
{
    if (jalign_ == J_MIDDLE) return -(line_width / 2.0);
    if (jalign_ == J_LEFT)   return -(layout_.width() / 2.0);
    if (jalign_ == J_RIGHT)  return (layout_.width() / 2.0) - line_width;
    return 0;
}

bool placement_finder::find_point_placement(pixel_position const& pos)
{
    glyph_positions_ptr glyphs = std::make_shared<glyph_positions>();

    /* Find text origin. */
    pixel_position displacement = scale_factor_ * info_->properties.displacement + alignment_offset();
    if (info_->properties.rotate_displacement) displacement = displacement.rotate(!orientation_);
    glyphs->set_base_point(pos + displacement);
    box2d<double> bbox;
    rotated_box2d(bbox, orientation_, layout_.width(), layout_.height());
    bbox.re_center(glyphs->get_base_point().x, glyphs->get_base_point().y);

    /* For point placements it is faster to just check the bounding box. */
    if (collision(bbox)) return false;
    /* add_marker first checks for collision and then updates the detector.*/
    if (has_marker_ && !add_marker(glyphs, pos)) return false;
    if (layout_.num_lines()) detector_.insert(bbox, layout_.text());

    /* IMPORTANT NOTE:
       x and y are relative to the center of the text
       coordinate system:
       x: grows from left to right
       y: grows from bottom to top (opposite of normal computer graphics)
    */
    double x, y;

    // set for upper left corner of text envelope for the first line, top left of first character
    y = layout_.height() / 2.0;
    glyphs->reserve(layout_.glyphs_count());

    for ( auto const& line : layout_)
    {
        y -= line.height(); //Automatically handles first line differently
        x = jalign_offset(line.width());

        for (auto const& glyph : line)
        {
            // place the character relative to the center of the string envelope
            glyphs->push_back(glyph, pixel_position(x, y).rotate(orientation_), orientation_);
            if (glyph.width)
            {
                //Only advance if glyph is not part of a multiple glyph sequence
                x += glyph.width + glyph.format->character_spacing * scale_factor_;
            }
        }
    }
    placements_.push_back(glyphs);
    return true;
}

template <typename T>
bool placement_finder::find_line_placements(T & path, bool points)
{
    if (!layout_.num_lines()) return true; //TODO
    vertex_cache pp(path);

    bool success = false;
    while (pp.next_subpath())
    {
        if (points)
        {
            if (pp.length() <= 0.001)
            {
                success = find_point_placement(pp.current_position()) || success;
                continue;
            }
        }
        else
        {
            if ((pp.length() < info_->properties.minimum_path_length * scale_factor_)
                ||
                (pp.length() <= 0.001) /* Clipping removed whole geometry */
                ||
                (pp.length() < layout_.width()))
                {
                    continue;
                }
        }

        double spacing = get_spacing(pp.length(), points ? 0. : layout_.width());

        horizontal_alignment_e halign = info_->properties.halign;
        if (halign == H_LEFT)
        {
            // Don't move
        }
        else if (halign == H_MIDDLE || halign == H_AUTO)
        {
            pp.forward(spacing/2.0);
        }
        else if (halign == H_RIGHT)
        {
            pp.forward(pp.length());
        }
        path_move_dx(pp);
        do
        {
            tolerance_iterator tolerance_offset(info_->properties.label_position_tolerance * scale_factor_, spacing); //TODO: Handle halign
            while (tolerance_offset.next())
            {
                vertex_cache::scoped_state state(pp);
                if (pp.move(tolerance_offset.get())
                    && (
                    (points && find_point_placement(pp.current_position()))
                    || (!points && single_line_placement(pp, info_->properties.upright))))
                {
                    success = true;
                    break;
                }
            }
        } while (pp.forward(spacing));
    }
    return success;
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


bool placement_finder::single_line_placement(vertex_cache &pp, text_upright_e orientation)
{
    /********************************************************************************
     * IMPORTANT NOTE: See note about coordinate systems in find_point_placement()! *
     ********************************************************************************/
    vertex_cache::scoped_state s(pp);

    glyph_positions_ptr glyphs = std::make_shared<glyph_positions>();
    std::vector<box2d<double> > bboxes;
    bboxes.reserve(layout_.text().length());
    int upside_down_glyph_count = 0;

    text_upright_e real_orientation = simplify_upright(orientation, pp.angle());

    double sign = (real_orientation == UPRIGHT_LEFT) ? -1 : 1;
    double offset = alignment_offset().y + info_->properties.displacement.y * scale_factor_ + sign * layout_.height()/2.;

    glyphs->reserve(layout_.glyphs_count());

    for (auto const& line : layout_)
    {
        //Only subtract half the line height here and half at the end because text is automatically
        //centered on the line
        offset -= sign * line.height()/2;
        vertex_cache & off_pp = pp.get_offseted(offset, sign*layout_.width());
        vertex_cache::scoped_state off_state(off_pp); //TODO: Remove this when a clean implementation in vertex_cache::get_offseted was done

        if (!off_pp.move(sign * jalign_offset(line.width()) - alignment_offset().x)) return false;

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
                if (!off_pp.move(sign * (layout_.cluster_width(current_cluster) + last_glyph_spacing)))
                {
                    return false;
                }
                current_cluster = glyph.char_index;
                last_glyph_spacing = glyph.format->character_spacing * scale_factor_;
                //Only calculate new angle at the start of each cluster!
                angle = normalize_angle(off_pp.angle(sign * layout_.cluster_width(current_cluster)));
                rot.init(angle);
                if ((info_->properties.max_char_angle_delta > 0) && (last_cluster_angle != 999) &&
                        std::fabs(normalize_angle(angle-last_cluster_angle)) > info_->properties.max_char_angle_delta)
                {
                    return false;
                }
                cluster_offset.clear();
                last_cluster_angle = angle;
            }
            if (std::abs(angle) > M_PI/2) ++upside_down_glyph_count;

            pixel_position pos = off_pp.current_position() + cluster_offset;
            //Center the text on the line
            double char_height = line.max_char_height();
            pos.y = -pos.y - char_height/2.0*rot.cos;
            pos.x =  pos.x + char_height/2.0*rot.sin;

            cluster_offset.x += rot.cos * glyph.width;
            cluster_offset.y -= rot.sin * glyph.width;

            box2d<double> bbox = get_bbox(glyph, pos, rot);
            if (collision(bbox)) return false;
            bboxes.push_back(bbox);
            glyphs->push_back(glyph, pos, rot);
        }
        //See comment above
        offset -= sign * line.height()/2;
    }
    if (upside_down_glyph_count > (layout_.text().length()/2))
    {
        if (orientation == UPRIGHT_AUTO)
        {
            //Try again with oposite orientation
            s.restore();
            return single_line_placement(pp, real_orientation == UPRIGHT_RIGHT ? UPRIGHT_LEFT : UPRIGHT_RIGHT);
        }
        //upright==left_only or right_only and more than 50% of characters upside down => no placement
        if (orientation == UPRIGHT_LEFT_ONLY || orientation == UPRIGHT_RIGHT_ONLY)
        {
            return false;
        }
    }
    for (box2d<double> const& bbox : bboxes)
    {
        detector_.insert(bbox, layout_.text());
    }
    placements_.push_back(glyphs);
    return true;
}

void placement_finder::path_move_dx(vertex_cache &pp)
{
    double dx = info_->properties.displacement.x * scale_factor_;
    if (dx != 0.0)
    {
        vertex_cache::state state = pp.save_state();
        if (!pp.move(dx)) pp.restore_state(state);
    }
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
    if (info_->properties.label_spacing > 0)
    {
        num_labels = static_cast<int>(floor(
            path_length / (info_->properties.label_spacing * scale_factor_ + layout_width)));
    }

    if (info_->properties.force_odd_labels && num_labels % 2 == 0)
    {
        --num_labels;
    }
    if (num_labels <= 0)
    {
        num_labels = 1;
    }
    return path_length / num_labels;
}

bool placement_finder::collision(const box2d<double> &box) const
{
    if (!detector_.extent().intersects(box)
            ||
        (info_->properties.avoid_edges && !extent_.contains(box))
            ||
        (info_->properties.minimum_padding > 0 &&
         !extent_.contains(box + (scale_factor_ * info_->properties.minimum_padding)))
            ||
        (!info_->properties.allow_overlap &&
         !detector_.has_point_placement(box, info_->properties.minimum_distance * scale_factor_))
        )
    {
        return true;
    }
    return false;
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
    if (collision(bbox)) return false;
    detector_.insert(bbox);
    return true;
}

box2d<double> placement_finder::get_bbox(glyph_info const& glyph, pixel_position const& pos, rotation const& rot)
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
    double width = layout_.cluster_width(glyph.char_index);
    if (glyph.width <= 0) width = -width;
    pixel_position tmp, tmp2;
    tmp.set(0, glyph.ymax);
    tmp = tmp.rotate(rot);
    tmp2.set(width, glyph.ymax);
    tmp2 = tmp2.rotate(rot);
    box2d<double> bbox(tmp.x,  -tmp.y,
                       tmp2.x, -tmp2.y);
    tmp.set(width, glyph.ymin);
    tmp = tmp.rotate(rot);
    bbox.expand_to_include(tmp.x, -tmp.y);
    tmp.set(0, glyph.ymin);
    tmp = tmp.rotate(rot);
    bbox.expand_to_include(tmp.x, -tmp.y);
    pixel_position pos2 = pos + pixel_position(0, glyph.offset.y).rotate(rot);
    bbox.move(pos2.x , -pos2.y);
    return bbox;
}


/*********************************************************************************************/


glyph_positions::glyph_positions()
    : data_(),
      base_point_(),
      marker_(),
      marker_pos_(),
      bbox_()
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


/*************************************************************************************/
typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
typedef coord_transform<CoordTransform,clipped_geometry_type> ClippedPathType;
typedef coord_transform<CoordTransform,geometry_type> PathType;
template bool placement_finder::find_line_placements<ClippedPathType>(ClippedPathType &, bool);
template bool placement_finder::find_line_placements<PathType>(PathType &, bool);


}// ns mapnik
