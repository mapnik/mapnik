/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#include <mapnik/text_placements/simple.hpp>
#include <mapnik/text_placements/list.hpp>
#include <mapnik/text_placements/dummy.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/formating/text.hpp>
#include <mapnik/processed_text.hpp>

#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/optional.hpp>

namespace mapnik {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
using boost::spirit::ascii::space;
using phoenix::push_back;
using phoenix::ref;
using qi::_1;
using boost::optional;

text_symbolizer_properties::text_symbolizer_properties() :
    label_placement(POINT_PLACEMENT),
    halign(H_AUTO),
    jalign(J_MIDDLE),
    valign(V_AUTO),
    label_spacing(0),
    label_position_tolerance(0),
    avoid_edges(false),
    minimum_distance(0.0),
    minimum_padding(0.0),
    max_char_angle_delta(22.5 * M_PI/180.0),
    force_odd_labels(false),
    allow_overlap(false),
    text_ratio(0),
    wrap_width(0),
    tree_()
{

}

void text_symbolizer_properties::process(processed_text &output, Feature const& feature) const
{
    output.clear();
    if (tree_) {
        tree_->apply(default_format, feature, output);
    } else {
#ifdef MAPNIK_DEBUG
        std::cerr << "Warning: text_symbolizer_properties can't produce text: No formating tree!\n";
#endif
    }
}

void text_symbolizer_properties::set_format_tree(formating::node_ptr tree)
{
    tree_ = tree;
}

formating::node_ptr text_symbolizer_properties::format_tree() const
{
    return tree_;
}

void text_symbolizer_properties::from_xml(boost::property_tree::ptree const &sym, std::map<std::string,font_set> const & fontsets)
{
    optional<label_placement_e> placement_ = get_opt_attr<label_placement_e>(sym, "placement");
    if (placement_) label_placement = *placement_;
    optional<vertical_alignment_e> valign_ = get_opt_attr<vertical_alignment_e>(sym, "vertical-alignment");
    if (valign_) valign = *valign_;
    optional<unsigned> text_ratio_ = get_opt_attr<unsigned>(sym, "text-ratio");
    if (text_ratio_) text_ratio = *text_ratio_;
    optional<unsigned> wrap_width_ = get_opt_attr<unsigned>(sym, "wrap-width");
    if (wrap_width_) wrap_width = *wrap_width_;
    optional<unsigned> label_position_tolerance_ = get_opt_attr<unsigned>(sym, "label-position-tolerance");
    if (label_position_tolerance_) label_position_tolerance = *label_position_tolerance_;
    optional<unsigned> spacing_ = get_opt_attr<unsigned>(sym, "spacing");
    if (spacing_) label_spacing = *spacing_;
    optional<unsigned> minimum_distance_ = get_opt_attr<unsigned>(sym, "minimum-distance");
    if (minimum_distance_) minimum_distance = *minimum_distance_;
    optional<unsigned> min_padding_ = get_opt_attr<unsigned>(sym, "minimum-padding");
    if (min_padding_) minimum_padding = *min_padding_;
    optional<unsigned> min_path_length_ = get_opt_attr<unsigned>(sym, "minimum-path-length");
    if (min_path_length_) minimum_path_length = *min_path_length_;
    optional<boolean> avoid_edges_ = get_opt_attr<boolean>(sym, "avoid-edges");
    if (avoid_edges_) avoid_edges = *avoid_edges_;
    optional<boolean> allow_overlap_ = get_opt_attr<boolean>(sym, "allow-overlap");
    if (allow_overlap_) allow_overlap = *allow_overlap_;
    optional<horizontal_alignment_e> halign_ = get_opt_attr<horizontal_alignment_e>(sym, "horizontal-alignment");
    if (halign_) halign = *halign_;
    optional<justify_alignment_e> jalign_ = get_opt_attr<justify_alignment_e>(sym, "justify-alignment");
    if (jalign_) jalign = *jalign_;
    /* Attributes needing special care */
    optional<std::string> orientation_ = get_opt_attr<std::string>(sym, "orientation");
    if (orientation_) orientation = parse_expression(*orientation_, "utf8");
    optional<double> dx = get_opt_attr<double>(sym, "dx");
    if (dx) displacement.first = *dx;
    optional<double> dy = get_opt_attr<double>(sym, "dy");
    if (dy) displacement.second = *dy;
    optional<double> max_char_angle_delta_ = get_opt_attr<double>(sym, "max-char-angle-delta");
    if (max_char_angle_delta_) max_char_angle_delta=(*max_char_angle_delta_)*(M_PI/180);

    optional<std::string> name_ = get_opt_attr<std::string>(sym, "name");
    if (name_) {
        std::clog << "### WARNING: Using 'name' in TextSymbolizer/ShieldSymbolizer is deprecated!\n";
        set_old_style_expression(parse_expression(*name_, "utf8"));
    }

    default_format.from_xml(sym, fontsets);
    formating::node_ptr n(formating::node::from_xml(sym));
    if (n) set_format_tree(n);
}

void text_symbolizer_properties::to_xml(boost::property_tree::ptree &node, bool explicit_defaults, text_symbolizer_properties const &dfl) const
{
    if (orientation)
    {
        const std::string & orientationstr = to_expression_string(*orientation);
        if (!dfl.orientation || orientationstr != to_expression_string(*(dfl.orientation)) || explicit_defaults) {
            set_attr(node, "orientation", orientationstr);
        }
    }

    if (displacement.first != dfl.displacement.first || explicit_defaults)
    {
        set_attr(node, "dx", displacement.first);
    }
    if (displacement.second != dfl.displacement.second || explicit_defaults)
    {
        set_attr(node, "dy", displacement.second);
    }
    if (label_placement != dfl.label_placement || explicit_defaults)
    {
        set_attr(node, "placement", label_placement);
    }
    if (valign != dfl.valign || explicit_defaults)
    {
        set_attr(node, "vertical-alignment", valign);
    }
    if (text_ratio != dfl.text_ratio || explicit_defaults)
    {
        set_attr(node, "text-ratio", text_ratio);
    }
    if (wrap_width != dfl.wrap_width || explicit_defaults)
    {
        set_attr(node, "wrap-width", wrap_width);
    }
    if (label_position_tolerance != dfl.label_position_tolerance || explicit_defaults)
    {
        set_attr(node, "label-position-tolerance", label_position_tolerance);
    }
    if (label_spacing != dfl.label_spacing || explicit_defaults)
    {
        set_attr(node, "spacing", label_spacing);
    }
    if (minimum_distance != dfl.minimum_distance || explicit_defaults)
    {
        set_attr(node, "minimum-distance", minimum_distance);
    }
    if (minimum_padding != dfl.minimum_padding || explicit_defaults)
    {
        set_attr(node, "minimum-padding", minimum_padding);
    }
    if (minimum_path_length != dfl.minimum_path_length || explicit_defaults)
    {
        set_attr(node, "minimum-path-length", minimum_path_length);
    }
    if (allow_overlap != dfl.allow_overlap || explicit_defaults)
    {
        set_attr(node, "allow-overlap", allow_overlap);
    }
    if (avoid_edges != dfl.avoid_edges || explicit_defaults)
    {
        set_attr(node, "avoid-edges", avoid_edges);
    }
    if (max_char_angle_delta != dfl.max_char_angle_delta || explicit_defaults)
    {
        set_attr(node, "max-char-angle-delta", max_char_angle_delta);
    }
    if (halign != dfl.halign || explicit_defaults)
    {
        set_attr(node, "horizontal-alignment", halign);
    }
    if (jalign != dfl.jalign || explicit_defaults)
    {
        set_attr(node, "justify-alignment", jalign);
    }
    if (valign != dfl.valign || explicit_defaults)
    {
        set_attr(node, "vertical-alignment", valign);
    }
    default_format.to_xml(node, explicit_defaults, dfl.default_format);
    if (tree_) tree_->to_xml(node);
}


void text_symbolizer_properties::add_expressions(expression_set &output) const
{
    output.insert(orientation);
    if (tree_) tree_->add_expressions(output);
}

void text_symbolizer_properties::set_old_style_expression(expression_ptr expr)
{
    tree_ = formating::node_ptr(new formating::text_node(expr));
}

char_properties::char_properties() :
    text_size(10.0),
    character_spacing(0),
    line_spacing(0),
    text_opacity(1.0),
    wrap_before(false),
    wrap_char(' '),
    text_transform(NONE),
    fill(color(0,0,0)),
    halo_fill(color(255,255,255)),
    halo_radius(0)
{

}

void char_properties::from_xml(boost::property_tree::ptree const &sym, std::map<std::string,font_set> const & fontsets)
{
    optional<double> text_size_ = get_opt_attr<double>(sym, "size");
    if (text_size_) text_size = *text_size_;
    optional<double> character_spacing_ = get_opt_attr<double>(sym, "character-spacing");
    if (character_spacing_) character_spacing = *character_spacing_;
    optional<color> fill_ = get_opt_attr<color>(sym, "fill");
    if (fill_) fill = *fill_;
    optional<color> halo_fill_ = get_opt_attr<color>(sym, "halo-fill");
    if (halo_fill_) halo_fill = *halo_fill_;
    optional<double> halo_radius_ = get_opt_attr<double>(sym, "halo-radius");
    if (halo_radius_) halo_radius = *halo_radius_;
    optional<boolean> wrap_before_ = get_opt_attr<boolean>(sym, "wrap-before");
    if (wrap_before_) wrap_before = *wrap_before_;
    optional<text_transform_e> tconvert_ = get_opt_attr<text_transform_e>(sym, "text-transform");
    if (tconvert_) text_transform = *tconvert_;
    optional<double> line_spacing_ = get_opt_attr<double>(sym, "line-spacing");
    if (line_spacing_) line_spacing = *line_spacing_;
    optional<double> opacity_ = get_opt_attr<double>(sym, "opacity");
    if (opacity_) text_opacity = *opacity_;
    optional<std::string> wrap_char_ = get_opt_attr<std::string>(sym, "wrap-character");
    if (wrap_char_ && (*wrap_char_).size() > 0) wrap_char = ((*wrap_char_)[0]);
    optional<std::string> face_name_ = get_opt_attr<std::string>(sym, "face-name");
    if (face_name_)
    {
        face_name = *face_name_;
    }
    optional<std::string> fontset_name_ = get_opt_attr<std::string>(sym, "fontset-name");
    if (fontset_name_) {
        std::map<std::string,font_set>::const_iterator itr = fontsets.find(*fontset_name_);
        if (itr != fontsets.end())
        {
            fontset = itr->second;
        } else
        {
            throw config_error("Unable to find any fontset named '" + *fontset_name_ + "'");
        }
    }
    if (!face_name.empty() && !fontset.get_name().empty())
    {
        throw config_error(std::string("Can't have both face-name and fontset-name"));
    }
    if (face_name.empty() && fontset.get_name().empty())
    {
        throw config_error(std::string("Must have face-name or fontset-name"));
    }
}

void char_properties::to_xml(boost::property_tree::ptree &node, bool explicit_defaults, char_properties const &dfl) const
{
    const std::string & fontset_name = fontset.get_name();
    const std::string & dfl_fontset_name = dfl.fontset.get_name();
    if (fontset_name != dfl_fontset_name || explicit_defaults)
    {
        set_attr(node, "fontset-name", fontset_name);
    }

    if (face_name != dfl.face_name || explicit_defaults)
    {
        set_attr(node, "face-name", face_name);
    }

    if (text_size != dfl.text_size || explicit_defaults)
    {
        set_attr(node, "size", text_size);
    }

    if (fill != dfl.fill || explicit_defaults)
    {
        set_attr(node, "fill", fill);
    }
    if (halo_radius != dfl.halo_radius || explicit_defaults)
    {
        set_attr(node, "halo-radius", halo_radius);
    }
    if (halo_fill != dfl.halo_fill || explicit_defaults)
    {
        set_attr(node, "halo-fill", halo_fill);
    }
    if (wrap_before != dfl.wrap_before || explicit_defaults)
    {
        set_attr(node, "wrap-before", wrap_before);
    }
    if (wrap_char != dfl.wrap_char || explicit_defaults)
    {
        set_attr(node, "wrap-character", std::string(1, wrap_char));
    }
    if (text_transform != dfl.text_transform || explicit_defaults)
    {
        set_attr(node, "text-transform", text_transform);
    }
    if (line_spacing != dfl.line_spacing || explicit_defaults)
    {
        set_attr(node, "line-spacing", line_spacing);
    }
    if (character_spacing != dfl.character_spacing || explicit_defaults)
    {
        set_attr(node, "character-spacing", character_spacing);
    }
    // for shield_symbolizer this is later overridden
    if (text_opacity != dfl.text_opacity || explicit_defaults)
    {
        set_attr(node, "opacity", text_opacity);
    }
}

/************************************************************************/

text_placements::text_placements() : properties()
{
}

void text_placements::add_expressions(expression_set &output)
{
    properties.add_expressions(output);
}


/************************************************************************/

text_placement_info::text_placement_info(text_placements const* parent,
                                         double scale_factor_, dimension_type dim, bool has_dimensions_)
    : properties(parent->properties),
      scale_factor(scale_factor_),
      has_dimensions(has_dimensions_),
      dimensions(dim),
      collect_extents(false)
{

}

bool text_placement_info_dummy::next()
{
    if (state) return false;
    state++;
    return true;
}

text_placement_info_ptr text_placements_dummy::get_placement_info(
    double scale_factor, dimension_type dim, bool has_dimensions) const
{
    return text_placement_info_ptr(new text_placement_info_dummy(
                                       this, scale_factor, dim, has_dimensions));
}

/************************************************************************/

bool text_placement_info_simple::next()
{
    while (1) {
        if (state > 0)
        {
            if (state > parent_->text_sizes_.size()) return false;
            properties.default_format.text_size = parent_->text_sizes_[state-1];
        }
        if (!next_position_only()) {
            state++;
            position_state = 0;
        } else {
            break;
        }
    }
    return true;
}

bool text_placement_info_simple::next_position_only()
{
    const position &pdisp = parent_->properties.displacement;
    position &displacement = properties.displacement;
    if (position_state >= parent_->direction_.size()) return false;
    directions_t dir = parent_->direction_[position_state];
    switch (dir) {
    case EXACT_POSITION:
        displacement = pdisp;
        break;
    case NORTH:
        displacement = std::make_pair(0, -abs(pdisp.second));
        break;
    case EAST:
        displacement = std::make_pair(abs(pdisp.first), 0);
        break;
    case SOUTH:
        displacement = std::make_pair(0, abs(pdisp.second));
        break;
    case WEST:
        displacement = std::make_pair(-abs(pdisp.first), 0);
        break;
    case NORTHEAST:
        displacement = std::make_pair(abs(pdisp.first), -abs(pdisp.second));
        break;
    case SOUTHEAST:
        displacement = std::make_pair(abs(pdisp.first), abs(pdisp.second));
        break;
    case NORTHWEST:
        displacement = std::make_pair(-abs(pdisp.first), -abs(pdisp.second));
        break;
    case SOUTHWEST:
        displacement = std::make_pair(-abs(pdisp.first), abs(pdisp.second));
        break;
    default:
        std::cerr << "WARNING: Unknown placement\n";
    }
    position_state++;
    return true;
}

text_placement_info_ptr text_placements_simple::get_placement_info(
    double scale_factor, dimension_type dim, bool has_dimensions) const
{
    return text_placement_info_ptr(new text_placement_info_simple(this,
                                                                  scale_factor, dim, has_dimensions));
}

/** Position string: [POS][SIZE]
 * [POS] is any combination of
 * N, E, S, W, NE, SE, NW, SW, X (exact position) (separated by commas)
 * [SIZE] is a list of font sizes, separated by commas. The first font size
 * is always the one given in the TextSymbolizer's parameters.
 * First all directions are tried, then font size is reduced
 * and all directions are tried again. The process ends when a placement is
 * found or the last fontsize is tried without success.
 * Example: N,S,15,10,8 (tries placement above, then below and if
 *    that fails it tries the additional font sizes 15, 10 and 8.
 */
void text_placements_simple::set_positions(std::string positions)
{
    positions_ = positions;
    struct direction_name_ : qi::symbols<char, directions_t>
    {
        direction_name_()
        {
            add
                ("N" , NORTH)
                ("E" , EAST)
                ("S" , SOUTH)
                ("W" , WEST)
                ("NE", NORTHEAST)
                ("SE", SOUTHEAST)
                ("NW", NORTHWEST)
                ("SW", SOUTHWEST)
                ("X" , EXACT_POSITION)
                ;
        }

    } direction_name;

    std::string::iterator first = positions.begin(),  last = positions.end();
    qi::phrase_parse(first, last,
                     (direction_name[push_back(phoenix::ref(direction_), _1)] % ',') >> *(',' >> qi::float_[push_back(phoenix::ref(text_sizes_), _1)]),
                     space
        );
    if (first != last) {
        std::cerr << "WARNING: Could not parse text_placement_simple placement string ('" << positions << "').\n";
    }
    if (direction_.size() == 0) {
        std::cerr << "WARNING: text_placements_simple with no valid placments! ('"<< positions<<"')\n";
    }
}

text_placements_simple::text_placements_simple()
{
    set_positions("X");
}

text_placements_simple::text_placements_simple(std::string positions)
{
    set_positions(positions);
}

std::string text_placements_simple::get_positions()
{
    return positions_; //TODO: Build string from data in direction_ and text_sizes_
}

/***************************************************************************/

bool text_placement_info_list::next()
{
    if (state == 0) {
        properties = parent_->properties;
    } else {
        if (state-1 >= parent_->list_.size()) return false;
        properties = parent_->list_[state-1];
    }
    state++;
    return true;
}

text_symbolizer_properties & text_placements_list::add()
{
    if (list_.size()) {
        text_symbolizer_properties &last = list_.back();
        list_.push_back(last); //Preinitialize with old values
    } else {
        list_.push_back(properties);
    }
    return list_.back();
}

text_symbolizer_properties & text_placements_list::get(unsigned i)
{
    return list_[i];
}

/***************************************************************************/

text_placement_info_ptr text_placements_list::get_placement_info(
    double scale_factor, dimension_type dim, bool has_dimensions) const
{
    return text_placement_info_ptr(new text_placement_info_list(this,
                                                                scale_factor, dim, has_dimensions));
}

text_placements_list::text_placements_list() : text_placements(), list_(0)
{

}

void text_placements_list::add_expressions(expression_set &output)
{
    properties.add_expressions(output);

    std::vector<text_symbolizer_properties>::const_iterator it;
    for (it=list_.begin(); it != list_.end(); it++)
    {
        it->add_expressions(output);
    }
}

unsigned text_placements_list::size() const
{
    return list_.size();
}


} //namespace
