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
#if 0
/** This class stores all format_runs in a line in left to right order.
 *
 * It can be used for rendering but no text processing (like line breaking)
 * should be done!
 * Glyphs are stored in runs with the same format.
 */
class text_line
{
public:
    text_line();
    std::vector<format_run_ptr> const& runs() const { return runs_; }
    void add_run(format_run_ptr run);
private:
    std::vector<format_run_ptr> runs_;
    double max_line_height; //Includes line spacing
    double max_text_height; //Height of the largest format run in this run.
};

typedef boost::shared_ptr<text_line> text_line_ptr;
#endif

class text_layout
{
public:
    text_layout(face_manager_freetype & font_manager);
    inline void add_text(UnicodeString const& str, char_properties_ptr format)
    {
        itemizer.add_text(str, format);
    }

    void break_lines(double break_width);
    void shape_text();
    void clear();
    unsigned size() const { return glyphs_.size(); }

    typedef std::vector<glyph_info> glyph_vector;
    glyph_vector const& get_glyphs() const { return glyphs_; }
    /** Get the text width. Returns 0 if shape_text() wasn't called before.
     * If break_lines was already called the width of the longest line is returned.
     **/
    double get_width() const { return width_; }

private:
    text_itemizer itemizer;
//    std::vector<text_line_ptr> lines_;
    /// Maps char index (UTF-16) to width. If multiple glyphs map to the same char the sum of all widths is used
    std::map<unsigned, double> width_map;
    glyph_vector glyphs_;
    face_manager_freetype &font_manager_;
    double total_width_;
    double width_;
};
}

#endif // TEXT_LAYOUT_HPP
