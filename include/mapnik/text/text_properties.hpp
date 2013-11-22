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
#ifndef TEXT_PROPERTIES_HPP
#define TEXT_PROPERTIES_HPP

// mapnik
#include <mapnik/text/char_properties_ptr.hpp>
#include <mapnik/color.hpp>
#include <mapnik/attribute.hpp>
#include <mapnik/value.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/font_set.hpp>
#include <mapnik/enumeration.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/text/formatting/base.hpp>
#include <mapnik/pixel_position.hpp>

// stl
#include <map>

// boost
#include <boost/optional.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

namespace mapnik
{

enum text_transform
{
    NONE = 0,
    UPPERCASE,
    LOWERCASE,
    CAPITALIZE,
    text_transform_MAX
};
DEFINE_ENUM(text_transform_e, text_transform);

typedef std::map<std::string, font_set> fontset_map;

struct MAPNIK_DECL char_properties
{
    char_properties();
    /** Construct object from XML. */
    void from_xml(xml_node const &sym, fontset_map const & fontsets);
    /** Write object to XML ptree. */
    void to_xml(boost::property_tree::ptree &node, bool explicit_defaults, char_properties const& dfl=char_properties()) const;
    std::string face_name;
    boost::optional<font_set> fontset;
    double text_size;
    double character_spacing;
    double line_spacing; //Largest total height (fontsize+line_spacing) per line is chosen
    double text_opacity;
    unsigned wrap_char;
    text_transform_e text_transform; //Per expression
    color fill;
    color halo_fill;
    double halo_radius;
};

enum label_placement_enum
{
    POINT_PLACEMENT,
    LINE_PLACEMENT,
    VERTEX_PLACEMENT,
    INTERIOR_PLACEMENT,
    label_placement_enum_MAX
};

DEFINE_ENUM(label_placement_e, label_placement_enum);

enum vertical_alignment
{
    V_TOP = 0,
    V_MIDDLE,
    V_BOTTOM,
    V_AUTO,
    vertical_alignment_MAX
};

DEFINE_ENUM(vertical_alignment_e, vertical_alignment);

enum horizontal_alignment
{
    H_LEFT = 0,
    H_MIDDLE,
    H_RIGHT,
    H_AUTO,
    horizontal_alignment_MAX
};

DEFINE_ENUM(horizontal_alignment_e, horizontal_alignment);

enum justify_alignment
{
    J_LEFT = 0,
    J_MIDDLE,
    J_RIGHT,
    J_AUTO,
    justify_alignment_MAX
};

DEFINE_ENUM(justify_alignment_e, justify_alignment);

enum text_upright
{
    UPRIGHT_AUTO,
    UPRIGHT_LEFT,
    UPRIGHT_RIGHT,
    UPRIGHT_LEFT_ONLY,
    UPRIGHT_RIGHT_ONLY,
    text_upright_MAX
};

DEFINE_ENUM(text_upright_e, text_upright);

class text_layout;


/** Contains all text symbolizer properties which are not directly related to text formatting. */
struct MAPNIK_DECL text_symbolizer_properties
{
    text_symbolizer_properties();
    /** Load all values from XML ptree. */
    void from_xml(xml_node const &sym, fontset_map const & fontsets);
    /** Save all values to XML ptree (but does not create a new parent node!). */
    void to_xml(boost::property_tree::ptree &node, bool explicit_defaults, text_symbolizer_properties const &dfl=text_symbolizer_properties()) const;

    /** Takes a feature and produces formated text as output.
     * The output object has to be created by the caller and passed in for thread safety.
     */
    void process(text_layout &output, feature_impl const& feature) const;
    /** Automatically create processing instructions for a single expression. */
    void set_old_style_expression(expression_ptr expr);
    /** Sets new format tree. */
    void set_format_tree(formatting::node_ptr tree);
    /** Get format tree. */
    formatting::node_ptr format_tree() const;
    /** Get a list of all expressions used in any placement.
     * This function is used to collect attributes. */
    void add_expressions(expression_set &output) const;

    //Per symbolizer options
    expression_ptr orientation;
    pixel_position displacement;
    label_placement_e label_placement;
    horizontal_alignment_e halign;
    justify_alignment_e jalign;
    vertical_alignment_e valign;
    /** distance between repeated labels on a single geometry */
    double label_spacing;
    /** distance the label can be moved on the line to fit, if 0 the default is used */
    double label_position_tolerance;
    bool avoid_edges;
    double minimum_distance;
    double minimum_padding;
    double minimum_path_length;
    double max_char_angle_delta;
    /** Always try render an odd amount of labels */
    bool force_odd_labels;
    bool allow_overlap;
    /** Only consider geometry with largest bbox (polygons) */
    bool largest_bbox_only;
    double text_ratio;
    double wrap_width;
    bool wrap_before;
    bool rotate_displacement;
    text_upright_e upright;
    /** Default values for char_properties. */
    char_properties_ptr format;
private:
    /** A tree of formatting::nodes which contain text and formatting information. */
    formatting::node_ptr tree_;
};

} //ns mapnik

#endif // TEXT_PROPERTIES_HPP
