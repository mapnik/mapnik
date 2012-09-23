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
text_layout::text_layout(face_manager_freetype &font_manager)
    : font_manager_(font_manager), itemizer_(), width_(0), height_(0), glyphs_count_(0),
      lines_()
{
}

void text_layout::add_text(const UnicodeString &str, char_properties_ptr format)
{
    itemizer_.add_text(str, format);
}

const UnicodeString &text_layout::get_text() const
{
    return itemizer_.get_text();
}

void text_layout::layout(double wrap_width, unsigned text_ratio, bool wrap_before)
{
    unsigned num_lines = itemizer_.num_lines();
    for (unsigned i = 0; i < num_lines; i++)
    {
        std::pair<unsigned, unsigned> line_limits = itemizer_.get_line(i);
        text_line_ptr line = boost::make_shared<text_line>(line_limits.first, line_limits.second);
        shape_text(line);
        break_line(line, wrap_width, text_ratio, wrap_before); //Break line if neccessary
    }
}


void text_layout::break_line(text_line_ptr line, double wrap_width, unsigned text_ratio, bool wrap_before)
{
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

    UnicodeString const& text = itemizer_.get_text();
    Locale locale; //TODO: Is the default constructor correct?
    UErrorCode status = U_ZERO_ERROR;
    BreakIterator *breakitr = BreakIterator::createLineInstance(locale, status);
    //Not breaking the text if an error occurs is probably the best thing we can do.
    if (!U_SUCCESS(status)) return;
    breakitr->setText(text);
    unsigned current_line_length = 0;
    unsigned last_break_position = 0;
    for (unsigned i=line->get_first_char(); i<line->get_last_char(); i++)
    {
        //TODO: Char spacing
        std::map<unsigned, double>::const_iterator width_itr = width_map_.find(i);
        if (width_itr != width_map_.end())
        {
            current_line_length += width_itr->second;
        }
        if (current_line_length > wrap_width)
        {
            unsigned break_position = wrap_before ? breakitr->preceding(i) : breakitr->following(i);
            if (break_position <= last_break_position || break_position == BreakIterator::DONE)
            {
                //A single word is longer than the maximum line width.
                //Violate line width requirement and choose next break position
                break_position = breakitr->following(i);
                if (break_position == BreakIterator::DONE)
                {
                    break_position = line->get_last_char();
                    MAPNIK_LOG_WARN(text_layout) << "Unexpected result in break_line. Trying to recover...\n";
                }
            }
            text_line_ptr new_line = boost::make_shared<text_line>(last_break_position, break_position);
            shape_text(new_line);
            add_line(new_line);
            last_break_position = break_position;
            i = break_position - 1;
            current_line_length = 0;
        }
    }
    if (last_break_position != line->get_last_char())
    {
        text_line_ptr new_line = boost::make_shared<text_line>(last_break_position, line->get_last_char());
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

void text_line::add_glyph(const glyph_info &glyph)
{
    line_height_ = std::max(line_height_, glyph.line_height + glyph.format->line_spacing);
    if (glyphs_.empty())
    {
        width_ = glyph.width;
    } else {
        width_ += glyph.width + glyphs_.back().format->character_spacing;
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

unsigned text_line::get_first_char() const
{
    return first_char_;
}

unsigned text_line::get_last_char() const
{
    return last_char_;
}

unsigned text_line::size() const
{
    return glyphs_.size();
}

} //ns mapnik
