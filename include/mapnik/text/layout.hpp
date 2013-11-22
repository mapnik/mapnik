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
#ifndef MAPNIK_TEXT_LAYOUT_HPP
#define MAPNIK_TEXT_LAYOUT_HPP

//mapnik
#include <mapnik/text/itemizer.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text/glyph_info.hpp>
#include <mapnik/text/char_properties_ptr.hpp>
#include <mapnik/text/harfbuzz_shaper.hpp>
#include <mapnik/text/icu_shaper.hpp>
#include <mapnik/text/dummy_shaper.hpp>

//stl
#include <vector>
#include <map>

namespace mapnik
{

class text_layout
{
public:
    typedef std::vector<text_line> line_vector;
    typedef line_vector::const_iterator const_iterator;
    typedef harfbuzz_shaper shaper_type;
    text_layout(face_manager_freetype & font_manager, double scale_factor);

    /** Adds a new text part. Call this function repeatedly to build the complete text. */
    void add_text(mapnik::value_unicode_string const& str, char_properties_ptr format);

    /** Returns the complete text stored in this layout.*/
    mapnik::value_unicode_string const& text() const;

    /** Processes the text into a list of glyphs, performing RTL/LTR handling, shaping and line breaking. */
    void layout(double wrap_width, unsigned text_ratio, bool wrap_before);

    /** Clear all data stored in this object. The object's state is the same as directly after construction. */
    void clear();

    // Height of all lines together (in pixels).
    inline double height() const { return height_; }
    // Width of the longest line (in pixels).
    inline double width() const { return width_ ; }

    // Line iterator.
    inline const_iterator begin() const { return lines_.begin(); }
    inline const_iterator end() const { return lines_.end(); }

    // Number of lines.
    inline std::size_t num_lines() const { return lines_.size(); }

    // Width of a certain glyph cluster (in pixels).
    inline double cluster_width(unsigned cluster) const
    {
        std::map<unsigned, double>::const_iterator width_itr = width_map_.find(cluster);
        if (width_itr != width_map_.end()) return width_itr->second;
        return 0;
    }

    // Returns the number of glyphs so memory can be preallocated.
    inline unsigned glyphs_count() const { return glyphs_count_;}

private:
    void break_line(text_line & line, double wrap_width, unsigned text_ratio, bool wrap_before);
    void shape_text(text_line & line);
    void add_line(text_line & line);
    void clear_cluster_widths(unsigned first, unsigned last);

    //input
    face_manager_freetype &font_manager_;
    double scale_factor_;

    //processing
    text_itemizer itemizer_;
    // Maps char index (UTF-16) to width. If multiple glyphs map to the same char the sum of all widths is used
    // note: this probably isn't the best solution. it would be better to have an object for each cluster, but
    // it needs to be implemented with no overhead.
    std::map<unsigned, double> width_map_;
    double width_;
    double height_;
    unsigned glyphs_count_;

    //output
    line_vector lines_;
};
}

#endif // TEXT_LAYOUT_HPP
