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

#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/text/harfbuzz_shaper.hpp>

// ICU
#include <unicode/brkiter.h>

namespace mapnik
{

// Output is centered around (0,0)
static void rotated_box2d(box2d<double> & box, rotation const& rot, pixel_position const& center, double width, double height)
{
    double half_width, half_height;
    if (rot.sin == 0 && rot.cos == 1.)
    {
        half_width  = width / 2.;
        half_height = height / 2.;
    }
    else
    {
        half_width  = (width * rot.cos + height * rot.sin) /2.;
        half_height = (width * rot.sin + height * rot.cos) /2.;
    }
    box.init(center.x - half_width, center.y - half_height, center.x + half_width, center.y + half_height);
}

pixel_position pixel_position::rotate(rotation const& rot) const
{
    return pixel_position(x * rot.cos - y * rot.sin, x * rot.sin + y * rot.cos);
}

text_layout::text_layout(face_manager_freetype & font_manager, double scale_factor, text_layout_properties const& properties)
    : font_manager_(font_manager),
      scale_factor_(scale_factor),
      itemizer_(),
      width_map_(),
      width_(0.0),
      height_(0.0),
      glyphs_count_(0),
      lines_(),
      properties_(properties) {}

void text_layout::add_text(mapnik::value_unicode_string const& str, evaluated_format_properties_ptr format)
{
    itemizer_.add_text(str, format);
}

void text_layout::add_child(text_layout_ptr const& child_layout)
{
    child_layout_list_.push_back(child_layout);
}

mapnik::value_unicode_string const& text_layout::text() const
{
    return itemizer_.text();
}

void text_layout::layout()
{
    unsigned num_lines = itemizer_.num_lines();
    for (unsigned i = 0; i < num_lines; ++i)
    {
        std::pair<unsigned, unsigned> line_limits = itemizer_.line(i);
        text_line line(line_limits.first, line_limits.second);
        //Break line if neccessary
        break_line(line, wrap_width_ * scale_factor_, text_ratio_, wrap_before_);
    }

    init_auto_alignment();

    // Find text origin.
    displacement_ = scale_factor_ * displacement_ + alignment_offset();
    if (rotate_displacement_) displacement_ = displacement_.rotate(!orientation_);
    // Find layout bounds, expanded for rotation
    rotated_box2d(bounds_, orientation_, displacement_, width_, height_);
}

// In the Unicode string characters are always stored in logical order.
// This makes line breaking easy. One word is added to the current line at a time. Once the line is too long
// we either go back one step or inset the line break at the current position (depending on "wrap_before" setting).
// At the end everything that is left over is added as the final line.
void text_layout::break_line(text_line & line, double wrap_width, unsigned text_ratio, bool wrap_before)
{
    shape_text(line);
    if (!wrap_width || line.width() < wrap_width)
    {
        add_line(line);
        return;

    }
    if (text_ratio)
    {
        double wrap_at;
        double string_width = line.width();
        double string_height = line.line_height();
        for (double i = 1.0; ((wrap_at = string_width/i)/(string_height*i)) > text_ratio && (string_width/i) > wrap_width; i += 1.0) ;
        wrap_width = wrap_at;
    }

    mapnik::value_unicode_string const& text = itemizer_.text();
    Locale locale; // TODO: Is the default constructor correct?
    UErrorCode status = U_ZERO_ERROR;
    BreakIterator *breakitr = BreakIterator::createLineInstance(locale, status);

    // Not breaking the text if an error occurs is probably the best thing we can do.
    // https://github.com/mapnik/mapnik/issues/2072
    if (!U_SUCCESS(status))
    {
        add_line(line);
        MAPNIK_LOG_ERROR(text_layout) << " could not create BreakIterator: " << u_errorName(status);
        return;
    }

    breakitr->setText(text);

    double current_line_length = 0;
    int last_break_position = static_cast<int>(line.first_char());
    for (unsigned i=line.first_char(); i < line.last_char(); ++i)
    {
        // TODO: character_spacing
        std::map<unsigned, double>::const_iterator width_itr = width_map_.find(i);
        if (width_itr != width_map_.end())
        {
            current_line_length += width_itr->second;
        }
        if (current_line_length <= wrap_width) continue;

        int break_position = wrap_before ? breakitr->preceding(i) : breakitr->following(i);
        // following() returns a break position after the last word. So DONE should only be returned
        // when calling preceding.
        if (break_position <= last_break_position || break_position == static_cast<int>(BreakIterator::DONE))
        {
            // A single word is longer than the maximum line width.
            // Violate line width requirement and choose next break position
            break_position = breakitr->following(i);
            if (break_position == static_cast<int>(BreakIterator::DONE))
            {
                break_position = line.last_char();
                MAPNIK_LOG_ERROR(text_layout) << "Unexpected result in break_line. Trying to recover...\n";
            }
        }
        // Break iterator operates on the whole string, while we only look at one line. So we need to
        // clamp break values.
        if (break_position < static_cast<int>(line.first_char()))
        {
            break_position = line.first_char();
        }
        if (break_position > static_cast<int>(line.last_char()))
        {
            break_position = line.last_char();
        }

        text_line new_line(last_break_position, break_position);
        clear_cluster_widths(last_break_position, break_position);
        shape_text(new_line);
        add_line(new_line);
        last_break_position = break_position;
        i = break_position - 1;
        current_line_length = 0;
    }
    if (last_break_position == static_cast<int>(line.first_char()))
    {
        // No line breaks => no reshaping required
        add_line(line);
    }
    else if (last_break_position != static_cast<int>(line.last_char()))
    {
        text_line new_line(last_break_position, line.last_char());
        clear_cluster_widths(last_break_position, line.last_char());
        shape_text(new_line);
        add_line(new_line);
    }
}

void text_layout::add_line(text_line & line)
{
    if (lines_.empty())
    {
        line.set_first_line(true);
    }
    height_ += line.height();
    glyphs_count_ += line.size();
    width_ = std::max(width_, line.width());
    lines_.push_back(line);
}

void text_layout::clear_cluster_widths(unsigned first, unsigned last)
{
    for (unsigned i=first; i<last; ++i)
    {
        width_map_[i] = 0;
    }
}

void text_layout::clear()
{
    itemizer_.clear();
    lines_.clear();
    width_map_.clear();
    width_ = 0.0;
    height_ = 0.0;
    child_layout_list_.clear();
}

void text_layout::shape_text(text_line & line)
{
    harfbuzz_shaper::shape_text(line, itemizer_, width_map_, font_manager_, scale_factor_);
}

void text_layout::evaluate_properties(feature_impl const& feature, attributes const& attrs)
{
    // dx,dy
    double dx = util::apply_visitor(extract_value<value_double>(feature,attrs), properties_.dx);
    double dy = util::apply_visitor(extract_value<value_double>(feature,attrs), properties_.dy);
    displacement_ = properties_.displacement_evaluator_(dx,dy);

    wrap_width_ = util::apply_visitor(extract_value<value_double>(feature,attrs), properties_.wrap_width);

    double angle = util::apply_visitor(extract_value<value_double>(feature,attrs), properties_.orientation);
    orientation_.init(angle * M_PI/ 180.0);

    wrap_before_ = util::apply_visitor(extract_value<value_bool>(feature,attrs), properties_.wrap_before);
    rotate_displacement_ = util::apply_visitor(extract_value<value_bool>(feature,attrs), properties_.rotate_displacement);

    valign_ = util::apply_visitor(extract_value<vertical_alignment_enum>(feature,attrs),properties_.valign);
    halign_ = util::apply_visitor(extract_value<horizontal_alignment_enum>(feature,attrs),properties_.halign);
    jalign_ = util::apply_visitor(extract_value<justify_alignment_enum>(feature,attrs),properties_.jalign);

}

void text_layout::init_auto_alignment()
{
    if (valign_ == V_AUTO)
    {
        if (displacement_.y > 0.0) valign_ = V_BOTTOM;
        else if (displacement_.y < 0.0) valign_ = V_TOP;
        else valign_ = V_MIDDLE;
    }
    if (halign_ == H_AUTO)
    {
        if (displacement_.x > 0.0) halign_ = H_RIGHT;
        else if (displacement_.x < 0.0) halign_ = H_LEFT;
        else halign_ = H_MIDDLE;
    }
    if (jalign_ == J_AUTO)
    {
        if (displacement_.x > 0.0) jalign_ = J_LEFT;
        else if (displacement_.x < 0.0) jalign_ = J_RIGHT;
        else jalign_ = J_MIDDLE;
    }
}

pixel_position text_layout::alignment_offset() const
{
    pixel_position result(0,0);
    // if needed, adjust for desired vertical alignment
    if (valign_ == V_TOP)
    {
        result.y = -0.5 * height();  // move center up by 1/2 the total height
    }
    else if (valign_ == V_BOTTOM)
    {
        result.y = 0.5 * height();  // move center down by the 1/2 the total height
    }
    // set horizontal position to middle of text
    if (halign_ == H_LEFT)
    {
        result.x = -0.5 * width();  // move center left by 1/2 the string width
    }
    else if (halign_ == H_RIGHT)
    {
        result.x = 0.5 * width();  // move center right by 1/2 the string width
    }
    return result;
}

double text_layout::jalign_offset(double line_width) const
{
    if (jalign_ == J_MIDDLE) return -(line_width / 2.0);
    if (jalign_ == J_LEFT)   return -(width() / 2.0);
    if (jalign_ == J_RIGHT)  return (width() / 2.0) - line_width;
    return 0;
}

void layout_container::add(text_layout_ptr layout)
{
    text_ += layout->text();
    layouts_.push_back(layout);

    for (text_layout_ptr const& child_layout : layout->get_child_layouts())
    {
        add(child_layout);
    }
}

void layout_container::layout()
{
    bounds_.init(0,0,0,0);
    glyphs_count_ = 0;
    line_count_ = 0;

    bool first = true;
    for (text_layout_ptr const& layout : layouts_)
    {
        layout->layout();

        glyphs_count_ += layout->glyphs_count();
        line_count_ += layout->num_lines();

        if (first)
        {
            bounds_ = layout->bounds();
            first = false;
        }
        else
        {
            bounds_.expand_to_include(layout->bounds());
        }
    }
}

void layout_container::clear()
{
    layouts_.clear();
    text_.remove();
    bounds_.init(0,0,0,0);
    glyphs_count_ = 0;
    line_count_ = 0;
}


} //ns mapnik
