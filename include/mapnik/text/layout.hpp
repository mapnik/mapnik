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

//boost
#include <boost/shared_ptr.hpp>

namespace mapnik
{

/** This class stores all glyphs in a format run (i.e. conscutive glyphs with the same format). */
class format_run
{
    char_properties_ptr properties;
    std::vector<glyph_info> const& glyphs() const { return glyphs_; }
    void add_glyph(glyph_info const& info);
private:
    std::vector<glyph_info> glyphs_;
};

typedef boost::shared_ptr<format_run> format_run_ptr;



/** This class stores all format_runs in a line in left to right order.
 *
 * It can be used for rendering but no text processing (like line breaking)
 * should be done!
 * Glyphs are stored in runs with the same format.
 */
class text_line
{
public:
    double max_text_height; //Height of the largest format run in this run.
    double max_line_height; //Includes line spacing
    std::vector<format_run_ptr> const& runs() const { return runs_; }
    void add_run(format_run_ptr);

private:
    std::vector<format_run_ptr> runs_;
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

    void break_lines();
    void shape_text();
    void clear();

private:
    text_itemizer itemizer;
    std::vector<text_line_ptr> lines_;
    std::vector<glyph_info> glyphs_;
    face_manager_freetype &font_manager_;
};
}

#endif // TEXT_LAYOUT_HPP
