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

#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/text/harfbuzz_shaper.hpp>
#include <mapnik/make_unique.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <unicode/brkiter.h>
#pragma GCC diagnostic pop

#include <algorithm>

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

pixel_position evaluate_displacement(double dx, double dy, directions_e dir)
{
    switch (dir)
    {
    case EXACT_POSITION:
        return pixel_position(dx,dy);
        break;
    case NORTH:
        return pixel_position(0,-std::abs(dy));
        break;
    case EAST:
        return pixel_position(std::abs(dx),0);
        break;
    case SOUTH:
        return pixel_position(0,std::abs(dy));
        break;
    case WEST:
        return pixel_position(-std::abs(dx),0);
        break;
    case NORTHEAST:
        return pixel_position(std::abs(dx),-std::abs(dy));
        break;
    case SOUTHEAST:
        return pixel_position(std::abs(dx),std::abs(dy));
        break;
    case NORTHWEST:
        return pixel_position(-std::abs(dx),-std::abs(dy));
        break;
    case SOUTHWEST:
        return pixel_position(-std::abs(dx),std::abs(dy));
        break;
    default:
        return pixel_position(dx,dy);
    }
}

pixel_position pixel_position::rotate(rotation const& rot) const
{
    return pixel_position(x * rot.cos - y * rot.sin, x * rot.sin + y * rot.cos);
}

text_layout::text_layout(face_manager_freetype & font_manager,
                         feature_impl const& feature,
                         attributes const& attrs,
                         double scale_factor,
                         text_symbolizer_properties const& properties,
                         text_layout_properties const& layout_defaults,
                         formatting::node_ptr tree)
    : font_manager_(font_manager),
      scale_factor_(scale_factor),
      itemizer_(),
      width_map_(),
      width_(0.0),
      height_(0.0),
      glyphs_count_(0),
      lines_(),
      layout_properties_(layout_defaults),
      properties_(properties),
      format_(std::make_unique<detail::evaluated_format_properties>())
    {
        double dx = util::apply_visitor(extract_value<value_double>(feature,attrs), layout_properties_.dx);
        double dy = util::apply_visitor(extract_value<value_double>(feature,attrs), layout_properties_.dy);
        displacement_ = evaluate_displacement(dx,dy, layout_properties_.dir);
        std::string wrap_str = util::apply_visitor(extract_value<std::string>(feature,attrs), layout_properties_.wrap_char);
        if (!wrap_str.empty()) wrap_char_ = wrap_str[0];
        wrap_width_ = util::apply_visitor(extract_value<value_double>(feature,attrs), layout_properties_.wrap_width);
        double angle = util::apply_visitor(extract_value<value_double>(feature,attrs), layout_properties_.orientation);
        orientation_.init(angle * M_PI/ 180.0);
        wrap_before_ = util::apply_visitor(extract_value<value_bool>(feature,attrs), layout_properties_.wrap_before);
        repeat_wrap_char_ = util::apply_visitor(extract_value<value_bool>(feature,attrs), layout_properties_.repeat_wrap_char);
        rotate_displacement_ = util::apply_visitor(extract_value<value_bool>(feature,attrs), layout_properties_.rotate_displacement);
        valign_ = util::apply_visitor(extract_value<vertical_alignment_enum>(feature,attrs),layout_properties_.valign);
        halign_ = util::apply_visitor(extract_value<horizontal_alignment_enum>(feature,attrs),layout_properties_.halign);
        jalign_ = util::apply_visitor(extract_value<justify_alignment_enum>(feature,attrs),layout_properties_.jalign);

        // Takes a feature and produces formatted text as output.
        if (tree)
        {
            format_properties const& format_defaults = properties_.format_defaults;
            format_->text_size = util::apply_visitor(extract_value<value_double>(feature,attrs), format_defaults.text_size);
            format_->character_spacing = util::apply_visitor(extract_value<value_double>(feature,attrs), format_defaults.character_spacing);
            format_->line_spacing = util::apply_visitor(extract_value<value_double>(feature,attrs), format_defaults.line_spacing);
            format_->text_opacity = util::apply_visitor(extract_value<value_double>(feature,attrs), format_defaults.text_opacity);
            format_->halo_opacity = util::apply_visitor(extract_value<value_double>(feature,attrs), format_defaults.halo_opacity);
            format_->halo_radius = util::apply_visitor(extract_value<value_double>(feature,attrs), format_defaults.halo_radius);
            format_->fill = util::apply_visitor(extract_value<color>(feature,attrs), format_defaults.fill);
            format_->halo_fill = util::apply_visitor(extract_value<color>(feature,attrs), format_defaults.halo_fill);
            format_->text_transform = util::apply_visitor(extract_value<text_transform_enum>(feature,attrs), format_defaults.text_transform);
            format_->face_name = format_defaults.face_name;
            format_->fontset = format_defaults.fontset;
            format_->ff_settings = util::apply_visitor(extract_value<font_feature_settings>(feature,attrs), format_defaults.ff_settings);
            // Turn off ligatures if character_spacing > 0.
            if (format_->character_spacing > .0 && format_->ff_settings.count() == 0)
            {
                format_->ff_settings.append(font_feature_liga_off);
            }
            tree->apply(format_, feature, attrs, *this);
        }
        else
        {
            MAPNIK_LOG_WARN(text_properties) << "text_symbolizer_properties can't produce text: No formatting tree!";
        }
    }

void text_layout::add_text(mapnik::value_unicode_string const& str, evaluated_format_properties_ptr const& format)
{
    itemizer_.add_text(str, format);
}

void text_layout::add_child(text_layout_ptr const& child_layout)
{
    child_layout_list_.push_back(child_layout);
}

evaluated_format_properties_ptr & text_layout::new_child_format_ptr(evaluated_format_properties_ptr const& p)
{
    format_ptrs_.emplace_back(std::make_unique<detail::evaluated_format_properties>(*p));
    return format_ptrs_.back();
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
        // Break line if neccessary
        if (wrap_char_ != ' ')
        {
            break_line(itemizer_.line(i));
        }
        else
        {
            break_line_icu(itemizer_.line(i));
        }
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
// we either go back one step or insert the line break at the current position (depending on "wrap_before" setting).
// At the end everything that is left over is added as the final line.
void text_layout::break_line_icu(std::pair<unsigned, unsigned> && line_limits)
{
    using BreakIterator = icu::BreakIterator;
    text_line line(line_limits.first, line_limits.second);
    shape_text(line);

    double scaled_wrap_width = wrap_width_ * scale_factor_;
    if (scaled_wrap_width <= 0 || line.width() < scaled_wrap_width)
    {
        add_line(std::move(line));
        return;
    }
    if (text_ratio_ > 0)
    {
        double wrap_at;
        double string_width = line.width();
        double string_height = line.line_height();
        for (double i = 1.0;
             ((wrap_at = string_width/i)/(string_height*i)) > text_ratio_ && (string_width/i) > scaled_wrap_width;
             i += 1.0) ;
        scaled_wrap_width = wrap_at;
    }

    mapnik::value_unicode_string const& text = itemizer_.text();
    icu::Locale locale; // TODO: Is the default constructor correct?
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<BreakIterator> breakitr(BreakIterator::createLineInstance(locale, status));

    // Not breaking the text if an error occurs is probably the best thing we can do.
    // https://github.com/mapnik/mapnik/issues/2072
    if (!U_SUCCESS(status))
    {
        add_line(std::move(line));
        MAPNIK_LOG_ERROR(text_layout) << " could not create BreakIterator: " << u_errorName(status);
        return;
    }

    breakitr->setText(text);
    double current_line_length = 0;
    int last_break_position = static_cast<int>(line.first_char());
    for (unsigned i = line.first_char(); i < line.last_char(); ++i)
    {
        // TODO: character_spacing
        std::map<unsigned, double>::const_iterator width_itr = width_map_.find(i);
        if (width_itr != width_map_.end())
        {
            current_line_length += width_itr->second;
        }
        if (current_line_length <= scaled_wrap_width) continue;

        int break_position = wrap_before_ ? breakitr->preceding(i + 1) : breakitr->following(i);
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
        bool adjust_for_space_character = break_position > 0 && text[break_position - 1] == 0x0020;

        text_line new_line(last_break_position, adjust_for_space_character ? break_position - 1 : break_position);
        clear_cluster_widths(last_break_position, adjust_for_space_character ? break_position - 1 : break_position);
        shape_text(new_line);
        add_line(std::move(new_line));

        last_break_position = break_position;
        i = break_position - 1;
        current_line_length = 0;
    }

    if (last_break_position == static_cast<int>(line.first_char()))
    {
        // No line breaks => no reshaping required
        add_line(std::move(line));
    }
    else if (last_break_position != static_cast<int>(line.last_char()))
    {
        text_line new_line(last_break_position, line.last_char());
        clear_cluster_widths(last_break_position, line.last_char());
        shape_text(new_line);
        add_line(std::move(new_line));
    }
}

struct line_breaker : util::noncopyable
{
    line_breaker(value_unicode_string const& ustr, char  wrap_char)
        : ustr_(ustr),
          wrap_char_(wrap_char) {}

    std::int32_t following(std::int32_t offset)
    {
        std::int32_t pos = ustr_.indexOf(wrap_char_, offset);
        if (pos != -1) ++pos;
        return pos;
    }

    std::int32_t preceding(std::int32_t offset)
    {
        std::int32_t pos = ustr_.lastIndexOf(wrap_char_, 0, offset);
        if (pos != -1) ++pos;
        return pos;
    }

    value_unicode_string const& ustr_;
    char wrap_char_;
};

inline int adjust_last_break_position (int pos, bool repeat_wrap_char)
{
    if (repeat_wrap_char)  return (pos==0) ? 0: pos - 1;
    else return pos;
}

void text_layout::break_line(std::pair<unsigned, unsigned> && line_limits)
{
    using BreakIterator = icu::BreakIterator;
    text_line line(line_limits.first, line_limits.second);
    shape_text(line);
    double scaled_wrap_width = wrap_width_ * scale_factor_;
    if (!scaled_wrap_width || line.width() < scaled_wrap_width)
    {
        add_line(std::move(line));
        return;

    }
    if (text_ratio_)
    {
        double wrap_at;
        double string_width = line.width();
        double string_height = line.line_height();
        for (double i = 1.0; ((wrap_at = string_width/i)/(string_height*i)) > text_ratio_ && (string_width/i) > scaled_wrap_width; i += 1.0) ;
        scaled_wrap_width = wrap_at;
    }
    mapnik::value_unicode_string const& text = itemizer_.text();
    line_breaker breaker(text, wrap_char_);
    double current_line_length = 0;
    int last_break_position = static_cast<int>(line.first_char());
    for (unsigned i=line.first_char(); i < line.last_char(); ++i)
    {
        std::map<unsigned, double>::const_iterator width_itr = width_map_.find(i);
        if (width_itr != width_map_.end())
        {
            current_line_length += width_itr->second;
        }
        if (current_line_length <= scaled_wrap_width) continue;

        int break_position = wrap_before_ ? breaker.preceding(i + 1) : breaker.following(i);
        if (break_position <= last_break_position || break_position == static_cast<int>(BreakIterator::DONE))
        {
            break_position = breaker.following(i);
            if (break_position == static_cast<int>(BreakIterator::DONE))
            {
                break_position = line.last_char();
            }
        }
        if (break_position < static_cast<int>(line.first_char()))
        {
            break_position = line.first_char();
        }
        if (break_position > static_cast<int>(line.last_char()))
        {
            break_position = line.last_char();
        }
        text_line new_line(adjust_last_break_position(last_break_position, repeat_wrap_char_), break_position);
        clear_cluster_widths(adjust_last_break_position(last_break_position, repeat_wrap_char_), break_position);
        shape_text(new_line);
        add_line(std::move(new_line));
        last_break_position = break_position;
        i = break_position - 1;
        current_line_length = 0;
    }
    if (last_break_position == static_cast<int>(line.first_char()))
    {
        add_line(std::move(line));
    }
    else if (last_break_position != static_cast<int>(line.last_char()))
    {
        text_line new_line(adjust_last_break_position(last_break_position, repeat_wrap_char_), line.last_char());
        clear_cluster_widths(adjust_last_break_position(last_break_position, repeat_wrap_char_), line.last_char());
        shape_text(new_line);
        add_line(std::move(new_line));
    }
}

void text_layout::add_line(text_line && line)
{
    if (lines_.empty())
    {
        line.set_first_line(true);
    }
    height_ += line.height();
    glyphs_count_ += line.size();
    width_ = std::max(width_, line.width());
    lines_.emplace_back(std::move(line));
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

text_layout::const_iterator text_layout::longest_line() const
{
    return std::max_element(lines_.begin(), lines_.end(),
        [](text_line const& line1, text_line const& line2)
        {
            return line1.glyphs_width() < line2.glyphs_width();
        });
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
