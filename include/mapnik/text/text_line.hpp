/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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
#ifndef MAPNIK_TEXT_LINE_HPP
#define MAPNIK_TEXT_LINE_HPP

//stl
#include <vector>
#include <mapnik/util/noncopyable.hpp>

namespace mapnik
{

struct glyph_info;

// This class stores all glyphs of a line in left to right order.
// It can be used for rendering but no text processing (like line breaking)
// should be done!

class text_line : util::noncopyable
{
public:
    using glyph_vector = std::vector<glyph_info>;
    using const_iterator = glyph_vector::const_iterator;

    text_line(unsigned first_char, unsigned last_char);

    text_line( text_line && rhs);

    // Append glyph.
    void add_glyph(glyph_info && glyph, double scale_factor_);

    // Preallocate memory.
    void reserve(glyph_vector::size_type length);
    // Iterator to first glyph.
    const_iterator begin() const;
    // Iterator beyond last glyph.
    const_iterator end() const;

    // Width of all glyphs including character spacing.
    double width() const { return width_; }
    // Width of all glyphs without character spacing.
    double glyphs_width() const { return glyphs_width_; }
    // Real line height. For first line: max_char_height(), for all others: line_height().
    double height() const;

    // Height of the tallest glyph in this line.
    double max_char_height() const { return max_char_height_; }

    // Called for each font/style to update the maximum height of this line.
    void update_max_char_height(double max_char_height);

    // Line height including line spacing.
    double line_height() const { return line_height_; }

    // Is this object is the first line of a multi-line text?
    // Used to exclude linespacing from first line's height.
    void set_first_line(bool first_line);

    // Index of first UTF-16 char.
    unsigned first_char() const;
    // Index of last UTF-16 char.
    unsigned last_char() const;

    // Number of glyphs.
    unsigned size() const;

    unsigned space_count() const { return space_count_; }

private:
    glyph_vector glyphs_;
    double line_height_; // Includes line spacing (returned by freetype)
    double max_char_height_; // Max height of any glyphs in line - calculated by shaper
    double width_;
    double glyphs_width_;
    unsigned first_char_;
    unsigned last_char_;
    bool first_line_;
    unsigned space_count_;
};

} //namespace mapnik

#endif // MAPNIK_TEXT_LINE_HPP
