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
#include <mapnik/text/layout.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/debug.hpp>

// ICU
#include <unicode/brkiter.h>

// boost
#include <boost/make_shared.hpp>

namespace mapnik
{
text_layout::text_layout(face_manager_freetype &font_manager, double scale_factor)
    : font_manager_(font_manager), scale_factor_(scale_factor),
      itemizer_(), width_(0), height_(0), glyphs_count_(0),
      lines_()
{
}

void text_layout::add_text(const UnicodeString &str, char_properties_ptr format)
{
    itemizer_.add_text(str, format);
}

const UnicodeString &text_layout::text() const
{
    return itemizer_.text();
}

void text_layout::layout(double wrap_width, unsigned text_ratio, bool wrap_before)
{
    unsigned num_lines = itemizer_.num_lines();
    for (unsigned i = 0; i < num_lines; i++)
    {
        std::pair<unsigned, unsigned> line_limits = itemizer_.line(i);
        text_line_ptr line = boost::make_shared<text_line>(line_limits.first, line_limits.second);
        break_line(line, wrap_width, text_ratio, wrap_before); //Break line if neccessary
    }
}

/* In the Unicode string characters are always stored in logical order.
 * This makes line breaking easy. One word is added to the current line at a time. Once the line is too long
 * we either go back one step or inset the line break at the current position (depending on "wrap_before" setting).
 * At the end everything that is left over is added as the final line. */
void text_layout::break_line(text_line_ptr line, double wrap_width, unsigned text_ratio, bool wrap_before)
{
    shape_text(line);
    if (!wrap_width || line->width() < wrap_width)
    {
        add_line(line);
        return;

    }
    if (text_ratio)
    {
        double wrap_at;
        double string_width = line->width();
        double string_height = line->line_height();
        for (double i = 1.0; ((wrap_at = string_width/i)/(string_height*i)) > text_ratio && (string_width/i) > wrap_width; i += 1.0) ;
        wrap_width = wrap_at;
    }

    UnicodeString const& text = itemizer_.text();
    Locale locale; //TODO: Is the default constructor correct?
    UErrorCode status = U_ZERO_ERROR;
    BreakIterator *breakitr = BreakIterator::createLineInstance(locale, status);

    //Not breaking the text if an error occurs is probably the best thing we can do.
    if (!U_SUCCESS(status)) {
        add_line(line);
        return;
    }

    breakitr->setText(text);

    double current_line_length = 0;
    unsigned last_break_position = line->first_char();
    for (unsigned i=line->first_char(); i<line->last_char(); i++)
    {
        //TODO: character_spacing
        std::map<unsigned, double>::const_iterator width_itr = width_map_.find(i);
        if (width_itr != width_map_.end())
        {
            current_line_length += width_itr->second;
        }
        if (current_line_length <= wrap_width) continue;
        /***********************************************/


        unsigned break_position = wrap_before ? breakitr->preceding(i) : breakitr->following(i);
        /* following() returns a break position after the last word. So DONE should only be returned
         * when calling preceding. */
        if (break_position <= last_break_position || break_position == BreakIterator::DONE)
        {
            //A single word is longer than the maximum line width.
            //Violate line width requirement and choose next break position
            break_position = breakitr->following(i);
            if (break_position == BreakIterator::DONE)
            {
                break_position = line->last_char();
                MAPNIK_LOG_WARN(text_layout) << "Unexpected result in break_line. Trying to recover...\n";
            }
        }
        /* Break iterator operates on the whole string, while we only look at one line. So we need to
         * clamp break values. */
        if (break_position < line->first_char()) break_position = line->first_char();
        if (break_position > line->last_char()) break_position = line->last_char();

        text_line_ptr new_line = boost::make_shared<text_line>(last_break_position, break_position);
        clear_cluster_widths(last_break_position, break_position);
        shape_text(new_line);
        add_line(new_line);
        last_break_position = break_position;
        i = break_position - 1;
        current_line_length = 0;
    }
    if (last_break_position == line->first_char())
    {
        //No line breaks => no reshaping required
        add_line(line);
    }
    else if (last_break_position != line->last_char())
    {
        text_line_ptr new_line = boost::make_shared<text_line>(last_break_position, line->last_char());
        clear_cluster_widths(last_break_position, line->last_char());
        shape_text(new_line);
        add_line(new_line);
    }
}



void text_layout::add_line(text_line_ptr line)
{
    if (lines_.empty())
    {
        line->set_first_line(true);
    }
    lines_.push_back(line);
    width_ = std::max(width_, line->width());
    height_ += line->height();
    glyphs_count_ += line->size();
}

void text_layout::clear_cluster_widths(unsigned first, unsigned last)
{
    for (int i=first; i<last; i++)
    {
        width_map_[i] = 0;
    }
}

void text_layout::clear()
{
    itemizer_.clear();
    lines_.clear();
    width_map_.clear();
    width_ = 0.;
    height_ = 0.;
}

double text_layout::height() const
{
    return height_;
}

double text_layout::width() const
{
    return width_;
}

text_layout::const_iterator text_layout::begin() const
{
    return lines_.begin();
}

text_layout::const_iterator text_layout::end() const
{
    return lines_.end();
}

unsigned text_layout::size() const
{
    return lines_.size();
}

double text_layout::cluster_width(unsigned cluster) const
{
    std::map<unsigned, double>::const_iterator width_itr = width_map_.find(cluster);
    if (width_itr != width_map_.end()) return width_itr->second;
    return 0;
}

unsigned text_layout::glyphs_count() const
{
    return glyphs_count_;
}

/*********************************************************************************************/

text_line::text_line(unsigned first_char, unsigned last_char)
    : glyphs_(), line_height_(0.), max_char_height_(0.),
      width_(0.), first_char_(first_char), last_char_(last_char),
      first_line_(false)
{
}

void text_line::add_glyph(const glyph_info &glyph, double scale_factor_)
{
    line_height_ = std::max(line_height_, glyph.line_height + glyph.format->line_spacing);
    if (glyphs_.empty())
    {
        width_ = glyph.width;
    } else if (glyph.width) {
        //Only add character spacing if the character is not a zero-width part of a cluster.
        width_ += glyph.width + glyphs_.back().format->character_spacing  * scale_factor_;
    }
    glyphs_.push_back(glyph);
}


void text_line::reserve(glyph_vector::size_type length)
{
    glyphs_.reserve(length);
}

text_line::const_iterator text_line::begin() const
{
    return glyphs_.begin();
}

text_line::const_iterator text_line::end() const
{
    return glyphs_.end();
}

double text_line::height() const
{
    if (first_line_) return max_char_height_;
    return line_height_;
}

void text_line::update_max_char_height(double max_char_height)
{
    max_char_height_ = std::max(max_char_height_, max_char_height);
}

void text_line::set_first_line(bool first_line)
{
    first_line_ = first_line;
}

unsigned text_line::first_char() const
{
    return first_char_;
}

unsigned text_line::last_char() const
{
    return last_char_;
}

unsigned text_line::size() const
{
    return glyphs_.size();
}

} //ns mapnik
