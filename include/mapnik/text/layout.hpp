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
#ifndef MAPNIK_TEXT_LAYOUT_HPP
#define MAPNIK_TEXT_LAYOUT_HPP

//mapnik
#include <mapnik/text/itemizer.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text/glyph_info.hpp>
#include <mapnik/text/char_properties_ptr.hpp>

//stl
#include <vector>
#include <list>
#include <map>

//boost
#include <boost/shared_ptr.hpp>

namespace mapnik
{
/** This class stores all glyphs of a line in left to right order.
 *
 * It can be used for rendering but no text processing (like line breaking)
 * should be done!
 */
class text_line
{
public:
    text_line(unsigned first_char, unsigned last_char);
    typedef std::vector<glyph_info> glyph_vector;
    glyph_vector const& get_glyphs() const { return glyphs_; }
    void add_glyph(glyph_info const& glyph);
    void reserve(glyph_vector::size_type length);
    double width() const { return width_; }
    double max_char_height() const { return max_char_height_; }
    void set_max_char_height(double max_char_height);
    double line_height() const { return line_height_; }
private:
    glyph_vector glyphs_;
    double line_height_; //Includes line spacing (returned by freetype)
    double max_char_height_; //Height of 'X' character of the largest font in this run. //TODO: Initialize this!
    double width_;
    unsigned first_char_;
    unsigned last_char_;
};

typedef boost::shared_ptr<text_line> text_line_ptr;

class text_layout
{
public:
    text_layout(face_manager_freetype & font_manager);
    inline void add_text(UnicodeString const& str, char_properties_ptr format)
    {
        itemizer.add_text(str, format);
    }

    void layout(double wrap_width, unsigned text_ratio);

    void clear();

private:
    void break_line(text_line_ptr line, double wrap_width, unsigned text_ratio);
    void shape_text(text_line_ptr line, unsigned start, unsigned end);

    //input
    face_manager_freetype &font_manager_;

    //processing
    text_itemizer itemizer;
    /// Maps char index (UTF-16) to width. If multiple glyphs map to the same char the sum of all widths is used
    std::map<unsigned, double> width_map;
    double total_width_;


    //output
    std::vector<text_line_ptr> lines_;
};
}

#endif // TEXT_LAYOUT_HPP
