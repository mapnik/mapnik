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

//mapnik
#include <mapnik/text_symbolizer.hpp>
// boost
#include <boost/scoped_ptr.hpp>

static const char * label_placement_strings[] = {
    "point",
    "line",
    "vertex",
    ""
};


IMPLEMENT_ENUM( mapnik::label_placement_e, label_placement_strings );

static const char * vertical_alignment_strings[] = {
    "top",
    "middle",
    "bottom",
    ""
};


IMPLEMENT_ENUM( mapnik::vertical_alignment_e, vertical_alignment_strings );

static const char * horizontal_alignment_strings[] = {
    "left",
    "middle",
    "right",
    ""
};


IMPLEMENT_ENUM( mapnik::horizontal_alignment_e, horizontal_alignment_strings );

static const char * justify_alignment_strings[] = {
    "left",
    "middle",
    "right",
    ""
};


IMPLEMENT_ENUM( mapnik::justify_alignment_e, justify_alignment_strings );

static const char * text_convert_strings[] = {
    "none",
    "toupper",
    "tolower",
    ""
};


IMPLEMENT_ENUM( mapnik::text_convert_e, text_convert_strings );


namespace mapnik
{
    text_symbolizer::text_symbolizer(std::string const& name, std::string const& face_name, unsigned size, color const& fill)
        : name_(name),
          face_name_(face_name),
          //fontset_(default_fontset),
          size_(size),
          text_ratio_(0),
          wrap_width_(0),
          wrap_char_(' '),
          text_convert_(NONE),
          line_spacing_(0),
          character_spacing_(0),
          label_spacing_(0),
          label_position_tolerance_(0),
          force_odd_labels_(false),
          max_char_angle_delta_(0.0),
          fill_(fill),
          halo_fill_(color(255,255,255)),
          halo_radius_(0),
          label_p_(POINT_PLACEMENT),
          valign_(MIDDLE),
          anchor_(0.0,0.5),
          displacement_(0.0,0.0),
          avoid_edges_(false),
          minimum_distance_(0.0),
          overlap_(false),
          opacity_(1.0),
          wrap_before_(false),
          halign_(H_MIDDLE),
          jalign_(J_MIDDLE) {}

    text_symbolizer::text_symbolizer(std::string const& name, unsigned size, color const& fill)
        : name_(name),
          //face_name_(""),
          //fontset_(default_fontset),
          size_(size),
          text_ratio_(0),
          wrap_width_(0),
          wrap_char_(' '),
          text_convert_(NONE),
          line_spacing_(0),
          character_spacing_(0),
          label_spacing_(0),
          label_position_tolerance_(0),
          force_odd_labels_(false),
          max_char_angle_delta_(0.0),
          fill_(fill),
          halo_fill_(color(255,255,255)),
          halo_radius_(0),
          label_p_(POINT_PLACEMENT),
          valign_(MIDDLE),
          anchor_(0.0,0.5),
          displacement_(0.0,0.0),
          avoid_edges_(false),
          minimum_distance_(0.0),
          overlap_(false),
          opacity_(1.0),
          wrap_before_(false),
          halign_(H_MIDDLE),
          jalign_(J_MIDDLE) {}

    text_symbolizer::text_symbolizer(text_symbolizer const& rhs)
        : name_(rhs.name_),
          face_name_(rhs.face_name_),
          fontset_(rhs.fontset_),
          size_(rhs.size_),
          text_ratio_(rhs.text_ratio_),
          wrap_width_(rhs.wrap_width_),
          wrap_char_(rhs.wrap_char_),
          text_convert_(rhs.text_convert_),
          line_spacing_(rhs.line_spacing_),
          character_spacing_(rhs.character_spacing_),
          label_spacing_(rhs.label_spacing_),
          label_position_tolerance_(rhs.label_position_tolerance_),
          force_odd_labels_(rhs.force_odd_labels_),
          max_char_angle_delta_(rhs.max_char_angle_delta_),
          fill_(rhs.fill_),
          halo_fill_(rhs.halo_fill_),
          halo_radius_(rhs.halo_radius_),
          label_p_(rhs.label_p_),
          valign_(rhs.valign_),
          anchor_(rhs.anchor_),
          displacement_(rhs.displacement_),
          avoid_edges_(rhs.avoid_edges_),
          minimum_distance_(rhs.minimum_distance_),
          overlap_(rhs.overlap_),
          opacity_(rhs.opacity_),
          wrap_before_(rhs.wrap_before_),
          halign_(rhs.halign_),
          jalign_(rhs.jalign_) {}

    text_symbolizer& text_symbolizer::operator=(text_symbolizer const& other)
    {
        if (this == &other)
            return *this;
        name_ = other.name_;
        face_name_ = other.face_name_;
        fontset_ = other.fontset_;
        size_ = other.size_;
        text_ratio_ = other.text_ratio_;
        wrap_width_ = other.wrap_width_;
        wrap_char_ = other.wrap_char_;
        text_convert_ = other.text_convert_;
        line_spacing_ = other.line_spacing_;
        character_spacing_ = other.character_spacing_;
        label_spacing_ = other.label_spacing_;
        label_position_tolerance_ = other.label_position_tolerance_;
        force_odd_labels_ = other.force_odd_labels_;
        max_char_angle_delta_ = other.max_char_angle_delta_;
        fill_ = other.fill_;
        halo_fill_ = other.halo_fill_;
        halo_radius_ = other.halo_radius_;
        label_p_ = other.label_p_;
        valign_ = other.valign_;
        anchor_ = other.anchor_;
        displacement_ = other.displacement_;
        avoid_edges_ = other.avoid_edges_;
        minimum_distance_ = other.minimum_distance_;
        overlap_ = other.overlap_;
        opacity_ = other.opacity_;
        wrap_before_ = other.wrap_before_;
        halign_ = other.halign_;
        jalign_ = other.jalign_;
        return *this;
    }

    std::string const&  text_symbolizer::get_name() const
    {
        return name_;
    }

    void text_symbolizer::set_name(std::string name)
    {
        name_ = name;
    }

    std::string const&  text_symbolizer::get_face_name() const
    {
        return face_name_;
    }

    void text_symbolizer::set_face_name(std::string face_name)
    {
        face_name_ = face_name;
    }

    void text_symbolizer::set_fontset(FontSet const& fontset)
    {
        fontset_ = fontset;
    }

    FontSet const& text_symbolizer::get_fontset() const
    {
        return fontset_;
    }

    unsigned  text_symbolizer::get_text_ratio() const
    {
        return text_ratio_;
    }

    void  text_symbolizer::set_text_ratio(unsigned ratio)
    {
        text_ratio_ = ratio;
    }

    unsigned  text_symbolizer::get_wrap_width() const
    {
        return wrap_width_;
    }

    void  text_symbolizer::set_wrap_width(unsigned width)
    {
        wrap_width_ = width;
    }

    bool  text_symbolizer::get_wrap_before() const
    {
        return wrap_before_;
    }

    void  text_symbolizer::set_wrap_before(bool wrap_before)
    {
        wrap_before_ = wrap_before;
    }

    unsigned char text_symbolizer::get_wrap_char() const
    {
        return wrap_char_;
    }

    std::string text_symbolizer::get_wrap_char_string() const
    {
        return std::string(1, wrap_char_);
    }

    void  text_symbolizer::set_wrap_char(unsigned char character)
    {
        wrap_char_ = character;
    }

    void  text_symbolizer::set_wrap_char_from_string(std::string const& character)
    {
        wrap_char_ = (character)[0];
    }

    text_convert_e  text_symbolizer::get_text_convert() const
    {
        return text_convert_;
    }

    void  text_symbolizer::set_text_convert(text_convert_e convert)
    {
        text_convert_ = convert;
    }

    unsigned  text_symbolizer::get_line_spacing() const
    {
        return line_spacing_;
    }

    void  text_symbolizer::set_line_spacing(unsigned spacing)
    {
        line_spacing_ = spacing;
    }

    unsigned  text_symbolizer::get_character_spacing() const
    {
        return character_spacing_;
    }

    void  text_symbolizer::set_character_spacing(unsigned spacing)
    {
        character_spacing_ = spacing;
    }

    unsigned  text_symbolizer::get_label_spacing() const
    {
        return label_spacing_;
    }

    void  text_symbolizer::set_label_spacing(unsigned spacing)
    {
        label_spacing_ = spacing;
    }

    unsigned  text_symbolizer::get_label_position_tolerance() const
    {
        return label_position_tolerance_;
    }

    void  text_symbolizer::set_label_position_tolerance(unsigned tolerance)
    {
        label_position_tolerance_ = tolerance;
    }

    bool  text_symbolizer::get_force_odd_labels() const
    {
        return force_odd_labels_;
    }

    void  text_symbolizer::set_force_odd_labels(bool force)
    {
        force_odd_labels_ = force;
    }

    double text_symbolizer::get_max_char_angle_delta() const
    {
        return max_char_angle_delta_;
    }

    void text_symbolizer::set_max_char_angle_delta(double angle)
    {
        max_char_angle_delta_ = angle;
    }

    void text_symbolizer::set_text_size(unsigned size)
    {
        size_ = size;
    }

    unsigned  text_symbolizer::get_text_size() const
    {
        return size_;
    }

    void text_symbolizer::set_fill(color const& fill)
    {
        fill_ = fill;
    }

    color const&  text_symbolizer::get_fill() const
    {
        return fill_;
    }

    void  text_symbolizer::set_halo_fill(color const& fill)
    {
        halo_fill_ = fill;
    }

    color const&  text_symbolizer::get_halo_fill() const
    {
        return halo_fill_;
    }

    void  text_symbolizer::set_halo_radius(unsigned radius)
    {
        halo_radius_ = radius;
    }

    unsigned  text_symbolizer::get_halo_radius() const
    {
        return halo_radius_;
    }

    void  text_symbolizer::set_label_placement(label_placement_e label_p)
    {
        label_p_ = label_p;
    }

    label_placement_e  text_symbolizer::get_label_placement() const
    {
        return label_p_;
    }

    void text_symbolizer::set_vertical_alignment(vertical_alignment_e valign)
    {
       valign_ = valign;
    }

    vertical_alignment_e text_symbolizer::get_vertical_alignment() const
    {
       return valign_;
    }

    void  text_symbolizer::set_anchor(double x, double y)
    {
        anchor_ = boost::make_tuple(x,y);
    }

    position const& text_symbolizer::get_anchor() const
    {
        return anchor_;
    }

    void  text_symbolizer::set_displacement(double x, double y)
    {
        displacement_ = boost::make_tuple(x,y);
    }

    position const& text_symbolizer::get_displacement() const
    {
        return displacement_;
    }

    bool text_symbolizer::get_avoid_edges() const
    {
        return avoid_edges_;
    }

    void text_symbolizer::set_avoid_edges(bool avoid)
    {
        avoid_edges_ = avoid;
    }

    double text_symbolizer::get_minimum_distance() const
    {
        return minimum_distance_;
    }

    void text_symbolizer::set_minimum_distance(double distance)
    {
        minimum_distance_ = distance;
    }

    void text_symbolizer::set_allow_overlap(bool overlap)
    {
       overlap_ = overlap;
    }

    bool text_symbolizer::get_allow_overlap() const
    {
      return overlap_;
    }

    void text_symbolizer::set_opacity(double opacity)
    {
       opacity_ = opacity;
    }

    double text_symbolizer::get_opacity() const
    {
      return opacity_;
    }

    void text_symbolizer::set_horizontal_alignment(horizontal_alignment_e halign)
    {
       halign_ = halign;
    }

    horizontal_alignment_e text_symbolizer::get_horizontal_alignment() const
    {
       return halign_;
    }

    void text_symbolizer::set_justify_alignment(justify_alignment_e jalign)
    {
       jalign_ = jalign;
    }

    justify_alignment_e text_symbolizer::get_justify_alignment() const
    {
       return jalign_;
    }
}
