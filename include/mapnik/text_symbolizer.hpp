/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id$

#ifndef MAPNIK_TEXT_SYMBOLIZER_HPP
#define MAPNIK_TEXT_SYMBOLIZER_HPP

// mapnik
#include <mapnik/color.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/text_placements.hpp>

// boost
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

// stl
#include <string>

namespace mapnik
{

struct MAPNIK_DECL text_symbolizer : public symbolizer_base
{
    text_symbolizer(expression_ptr name, std::string const& face_name,
                    unsigned size, color const& fill,
                    text_placements_ptr placements = text_placements_ptr(
                        boost::make_shared<text_placements_dummy>())
                    );
    text_symbolizer(expression_ptr name, unsigned size, color const& fill,
                    text_placements_ptr placements = text_placements_ptr(
                        boost::make_shared<text_placements_dummy>())
                    );
    text_symbolizer(text_symbolizer const& rhs);
    text_symbolizer& operator=(text_symbolizer const& rhs);
    expression_ptr get_name() const;
    void set_name(expression_ptr expr);

    expression_ptr get_orientation() const; // orienation (rotation angle atm)
    void set_orientation(expression_ptr expr);
         
    unsigned get_text_ratio() const; // target ratio for text bounding box in pixels
    void set_text_ratio(unsigned ratio);
    unsigned get_wrap_width() const; // width to wrap text at, or trigger ratio
    void set_wrap_width(unsigned ratio);
    unsigned char get_wrap_char() const; // character used to wrap lines
    std::string get_wrap_char_string() const; // character used to wrap lines as std::string
    void set_wrap_char(unsigned char character);
    void set_wrap_char_from_string(std::string const& character);
    text_transform_e get_text_transform() const; // text conversion on strings before display
    void set_text_transform(text_transform_e convert);
    unsigned get_line_spacing() const; // spacing between lines of text
    void set_line_spacing(unsigned spacing);
    unsigned get_character_spacing() const; // spacing between characters in text
    void set_character_spacing(unsigned spacing);
    unsigned get_label_spacing() const; // spacing between repeated labels on lines
    void set_label_spacing(unsigned spacing);
    unsigned get_label_position_tolerance() const; //distance the label can be moved on the line to fit, if 0 the default is used
    void set_label_position_tolerance(unsigned tolerance);
    bool get_force_odd_labels() const; // try render an odd amount of labels
    void set_force_odd_labels(bool force);
    double get_max_char_angle_delta() const; // maximum change in angle between adjacent characters
    void set_max_char_angle_delta(double angle);
    unsigned get_text_size() const;
    void set_text_size(unsigned size);
    std::string const& get_face_name() const;
    void set_face_name(std::string face_name);
    font_set const& get_fontset() const;
    void set_fontset(font_set const& fset);
    color const& get_fill() const;
    void set_fill(color const& fill);
    void set_halo_fill(color const& fill);
    color const& get_halo_fill() const;
    void set_halo_radius(double radius);
    double get_halo_radius() const;
    void set_label_placement(label_placement_e label_p);
    label_placement_e get_label_placement() const;
    void set_vertical_alignment(vertical_alignment_e valign);
    vertical_alignment_e get_vertical_alignment() const;
    void set_anchor(double x, double y);
    position const& get_anchor() const;
    void set_displacement(double x, double y);
    position const& get_displacement() const;
    void set_avoid_edges(bool avoid);
    bool get_avoid_edges() const;
    void set_minimum_distance(double distance);
    double get_minimum_distance() const;
    void set_minimum_padding(double distance);
    double get_minimum_padding() const;
    void set_minimum_path_length(double size);
    double get_minimum_path_length() const;
    void set_allow_overlap(bool overlap);
    bool get_allow_overlap() const;
    void set_text_opacity(double opacity);
    double get_text_opacity() const;
    bool get_wrap_before() const; // wrap text at wrap_char immediately before current work
    void set_wrap_before(bool wrap_before);
    void set_horizontal_alignment(horizontal_alignment_e valign);
    horizontal_alignment_e get_horizontal_alignment() const;
    void set_justify_alignment(justify_alignment_e valign);
    justify_alignment_e get_justify_alignment() const;
    text_placements_ptr get_placement_options() const;
    void set_placement_options(text_placements_ptr placement_options);

private:
    expression_ptr name_;
    expression_ptr orientation_;
    std::string face_name_;
    font_set fontset_;
    unsigned text_ratio_;
    unsigned wrap_width_;
    unsigned char wrap_char_;
    text_transform_e text_transform_;
    unsigned line_spacing_;
    unsigned character_spacing_;
    unsigned label_spacing_;
    unsigned label_position_tolerance_;
    bool force_odd_labels_;
    double max_char_angle_delta_;
    color fill_;
    color halo_fill_;
    double halo_radius_;
    label_placement_e label_p_;
    position anchor_;
    bool avoid_edges_;
    double minimum_distance_;
    double minimum_padding_;
    double minimum_path_length_;
    bool overlap_;
    double text_opacity_;
    bool wrap_before_;
    text_placements_ptr placement_options_;
};
}

#endif //MAPNIK_TEXT_SYMBOLIZER_HPP
