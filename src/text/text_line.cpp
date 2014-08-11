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

#include <mapnik/text/text_line.hpp>
#include <mapnik/text/glyph_info.hpp>
#include <mapnik/text/text_properties.hpp>

namespace mapnik {

text_line::text_line(unsigned first_char, unsigned last_char)
    : glyphs_(),
      line_height_(0.0),
      max_char_height_(0.0),
      width_(0.0),
      first_char_(first_char),
      last_char_(last_char),
      first_line_(false)
{}

void text_line::add_glyph(glyph_info const& glyph, double scale_factor_)
{
    line_height_ = std::max(line_height_, glyph.line_height() + glyph.format->line_spacing);
    double advance = glyph.advance();
    if (glyphs_.empty())
    {
        width_ = advance;
    }
    else if (advance)
    {
        // Only add character spacing if the character is not a zero-width part of a cluster.
        width_ += advance + glyphs_.back().format->character_spacing  * scale_factor_;
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

} // end namespace mapnik
