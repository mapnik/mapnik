/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include <mapnik/value/types.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/geometry/box2d.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/text_line.hpp>
#include <mapnik/text/itemizer.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text/evaluated_format_properties_ptr.hpp>
#include <mapnik/text/rotation.hpp>

//stl
#include <vector>
#include <deque>
#include <memory>
#include <map>
#include <utility>

namespace mapnik
{

class feature_impl;
class text_layout;

using text_layout_ptr = std::shared_ptr<text_layout>;
using text_layout_vector = std::vector<text_layout_ptr>;
// this is a std::deque to ensure pointers stay valid as a deque
// "never invalidates pointers or references to the rest of the elements"
// http://en.cppreference.com/w/cpp/container/deque
// If this were a vector this test would crash:
// python tests/visual_tests/test.py text-expressionformat-color
using child_format_ptrs = std::deque<evaluated_format_properties_ptr>;

class text_layout
{
public:
    using line_vector = std::vector<text_line>;
    using const_iterator = line_vector::const_iterator;
    using child_iterator = text_layout_vector::const_iterator;

    text_layout(face_manager_freetype & font_manager,
                feature_impl const& feature,
                attributes const& attrs,
                double scale_factor,
                text_symbolizer_properties const& properties,
                text_layout_properties const& layout_defaults,
                formatting::node_ptr tree);

    // Adds a new text part. Call this function repeatedly to build the complete text.
    void add_text(mapnik::value_unicode_string const& str, evaluated_format_properties_ptr const& format);

    // Returns the complete text stored in this layout.
    mapnik::value_unicode_string const& text() const;

    // Processes the text into a list of glyphs, performing RTL/LTR handling, shaping and line breaking.
    void layout();

    // Clear all data stored in this object. The object's state is the same as directly after construction.
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

    void add_child(text_layout_ptr const& child_layout);

    inline text_layout_vector const& get_child_layouts() const { return child_layout_list_; }
    inline face_manager_freetype & get_font_manager() const { return font_manager_; }
    inline double get_scale_factor() const { return scale_factor_; }
    inline text_symbolizer_properties const& get_default_text_properties() const { return properties_; }
    inline text_layout_properties const& get_layout_properties() const { return layout_properties_; }

    inline rotation const& orientation() const { return orientation_; }
    inline pixel_position const& displacement() const { return displacement_; }
    inline box2d<double> const& bounds() const { return bounds_; }
    inline horizontal_alignment_e horizontal_alignment() const { return halign_; }
    pixel_position alignment_offset() const;
    double jalign_offset(double line_width) const;
    evaluated_format_properties_ptr & new_child_format_ptr(evaluated_format_properties_ptr const& p);

    const_iterator longest_line() const;

private:
    void break_line(std::pair<unsigned, unsigned> && line_limits);
    void break_line_icu(std::pair<unsigned, unsigned> && line_limits);
    void shape_text(text_line & line);
    void add_line(text_line && line);
    void clear_cluster_widths(unsigned first, unsigned last);
    void init_auto_alignment();

    // input
    face_manager_freetype & font_manager_;
    double scale_factor_;

    // processing
    text_itemizer itemizer_;
    // Maps char index (UTF-16) to width. If multiple glyphs map to the same char the sum of all widths is used
    // note: this probably isn't the best solution. it would be better to have an object for each cluster, but
    // it needs to be implemented with no overhead.
    std::map<unsigned, double> width_map_;
    double width_ = 0.0;
    double height_ = 0.0;
    unsigned glyphs_count_;

    // output
    line_vector lines_;

    // layout properties (owned by text_layout)
    text_layout_properties layout_properties_;

    // text symbolizer properties (owned by placement_finder's 'text_placement_info' (info_) which is owned by symbolizer_helper
    text_symbolizer_properties const& properties_;

    // format properties (owned by text_layout)
    evaluated_format_properties_ptr format_;

    // alignments
    vertical_alignment_e valign_;
    horizontal_alignment_e halign_;
    justify_alignment_e jalign_;

    // Precalculated values for maximum performance
    rotation orientation_ = {0,1.0};
    char wrap_char_ = ' ';
    double wrap_width_ = 0.0;
    bool wrap_before_ = false;
    bool repeat_wrap_char_ = false;
    bool rotate_displacement_ = false;
    double text_ratio_ = 0.0;
    pixel_position displacement_ = {0,0};
    box2d<double> bounds_ = {0, 0, 0, 0};

    // children
    text_layout_vector child_layout_list_;

    // take ownership of evaluated_format_properties_ptr of any format children
    // in order to keep them in scope
    // NOTE: this must not be a std::vector - see note above about child_format_ptrs
    child_format_ptrs format_ptrs_;
};

class layout_container
{
public:
    layout_container()
        : glyphs_count_(0), line_count_(0) {}

    void add(text_layout_ptr layout);
    void clear();

    void layout();

    inline size_t size() const { return layouts_.size(); }
    inline bool empty() const { return layouts_.empty(); }

    inline text_layout_vector::const_iterator begin() const { return layouts_.begin(); }
    inline text_layout_vector::const_iterator end() const { return layouts_.end(); }
    inline text_layout_ptr const& back() const { return layouts_.back(); }
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
