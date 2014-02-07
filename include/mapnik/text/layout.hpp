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
#include <mapnik/text/rotation.hpp>

//stl
#include <vector>
#include <map>

namespace mapnik
{

typedef std::shared_ptr<text_layout> text_layout_ptr;
typedef std::vector<text_layout_ptr> text_layout_vector;

class text_layout
{
public:
    typedef std::vector<text_line> line_vector;
    typedef line_vector::const_iterator const_iterator;
    typedef text_layout_vector::const_iterator child_iterator;
    typedef harfbuzz_shaper shaper_type;
    text_layout(face_manager_freetype & font_manager, double scale_factor, text_layout_properties_ptr properties);

    /** Adds a new text part. Call this function repeatedly to build the complete text. */
    void add_text(mapnik::value_unicode_string const& str, char_properties_ptr format);

    /** Returns the complete text stored in this layout.*/
    mapnik::value_unicode_string const& text() const;

    /** Processes the text into a list of glyphs, performing RTL/LTR handling, shaping and line breaking. */
    void layout();

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

    void add_child(text_layout_ptr child_layout);

    inline const text_layout_vector &get_child_layouts() const { return child_layout_list_; }

    inline face_manager<freetype_engine> &get_font_manager() const { return font_manager_; }
    inline double get_scale_factor() const { return scale_factor_; }
    inline text_layout_properties_ptr get_layout_properties() const { return properties_; }

    inline rotation const& orientation() const { return orientation_; }
    inline pixel_position const& displacement() const { return displacement_; }
    inline box2d<double> const& bounds() const { return bounds_; }

    pixel_position alignment_offset() const;
    double jalign_offset(double line_width) const;

    void init_orientation(feature_impl const& feature);

private:
    void break_line(text_line & line, double wrap_width, unsigned text_ratio, bool wrap_before);
    void shape_text(text_line & line);
    void add_line(text_line & line);
    void clear_cluster_widths(unsigned first, unsigned last);
    void init_alignment();

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

    //text layout properties
    text_layout_properties_ptr properties_;

    //alignments
    vertical_alignment_e valign_;
    horizontal_alignment_e halign_;
    justify_alignment_e jalign_;

    // Precalculated values for maximum performance
    rotation orientation_;
    pixel_position displacement_;
    box2d<double> bounds_;

    //children
    text_layout_vector child_layout_list_;
};

class layout_container
{
public:
    layout_container() : glyphs_count_(0), line_count_(0) {}

    void add(text_layout_ptr layout);
    void clear();

    void layout();

    inline size_t size() const { return layouts_.size(); }

    inline text_layout_vector::const_iterator begin() const { return layouts_.begin(); }
    inline text_layout_vector::const_iterator end() const { return layouts_.end(); }

    inline mapnik::value_unicode_string const& text() const { return text_; }

    inline unsigned glyphs_count() const { return glyphs_count_; }
    inline unsigned line_count() const { return line_count_; }

    inline box2d<double> const& bounds() const { return bounds_; }

    inline double width() const { return bounds_.width(); }
    inline double height() const { return bounds_.height(); }

private:
    text_layout_vector layouts_;

    mapnik::value_unicode_string text_;

    unsigned glyphs_count_;
    unsigned line_count_;

    box2d<double> bounds_;
};

}

#endif // TEXT_LAYOUT_HPP
