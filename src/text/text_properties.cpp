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
// mapnik
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/text/formatting/text.hpp>
#include <mapnik/xml_node.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/text/properties_util.hpp>
#include <mapnik/boolean.hpp>

// boost
#include <boost/property_tree/ptree.hpp>

namespace mapnik
{
using boost::optional;

text_symbolizer_properties::text_symbolizer_properties()
    : label_placement(POINT_PLACEMENT),
      label_spacing(0.0),
      label_position_tolerance(0.0),
      avoid_edges(false),
      text_margin(0.0),
      repeat_distance(0.0),
      minimum_distance(0.0),
      minimum_padding(0.0),
      minimum_path_length(0.0),
      max_char_angle_delta(22.5 * M_PI/180.0),
      allow_overlap(false),
      largest_bbox_only(true),
      upright(UPRIGHT_AUTO),
      layout_defaults(),
      format_defaults(),
      tree_() {}


void text_symbolizer_properties::evaluate_text_properties(feature_impl const& feature, attributes const& attrs)
{
    label_placement = util::apply_visitor(extract_value<label_placement_enum>(feature,attrs), expressions.label_placement);
    label_spacing = util::apply_visitor(extract_value<value_double>(feature,attrs), expressions.label_spacing);
    label_position_tolerance = util::apply_visitor(extract_value<value_double>(feature,attrs), expressions.label_position_tolerance);
    avoid_edges = util::apply_visitor(extract_value<value_bool>(feature,attrs), expressions.avoid_edges);
    text_margin = util::apply_visitor(extract_value<value_double>(feature,attrs), expressions.text_margin);
    repeat_distance = util::apply_visitor(extract_value<value_double>(feature,attrs), expressions.repeat_distance);
    minimum_distance = util::apply_visitor(extract_value<value_double>(feature,attrs), expressions.minimum_distance);
    minimum_padding = util::apply_visitor(extract_value<value_double>(feature,attrs), expressions.minimum_padding);
    minimum_path_length = util::apply_visitor(extract_value<value_double>(feature,attrs), expressions.minimum_path_length);
    max_char_angle_delta = util::apply_visitor(extract_value<value_double>(feature,attrs), expressions.max_char_angle_delta) * M_PI/180;
    allow_overlap = util::apply_visitor(extract_value<value_bool>(feature,attrs), expressions.allow_overlap);
    largest_bbox_only = util::apply_visitor(extract_value<value_bool>(feature,attrs), expressions.largest_bbox_only);
    upright = util::apply_visitor(extract_value<text_upright_enum>(feature,attrs), expressions.upright);
}

void text_symbolizer_properties::process(text_layout & output, feature_impl const& feature, attributes const& attrs) //const
{
    output.clear();

    if (tree_)
    {
        evaluate_text_properties(feature, attrs);
        //evaluate format properties
        evaluated_format_properties_ptr format = std::make_shared<detail::evaluated_format_properties>();

        format->text_size = util::apply_visitor(extract_value<value_double>(feature,attrs), format_defaults.text_size);
        format->character_spacing = util::apply_visitor(extract_value<value_double>(feature,attrs), format_defaults.character_spacing);
        format->line_spacing = util::apply_visitor(extract_value<value_double>(feature,attrs), format_defaults.line_spacing);
        format->text_opacity = util::apply_visitor(extract_value<value_double>(feature,attrs), format_defaults.text_opacity);
        format->halo_opacity = util::apply_visitor(extract_value<value_double>(feature,attrs), format_defaults.halo_opacity);
        format->halo_radius = util::apply_visitor(extract_value<value_double>(feature,attrs), format_defaults.halo_radius);
        format->fill = util::apply_visitor(extract_value<color>(feature,attrs), format_defaults.fill);
        format->halo_fill = util::apply_visitor(extract_value<color>(feature,attrs), format_defaults.halo_fill);
        format->text_transform = util::apply_visitor(extract_value<text_transform_enum>(feature,attrs), format_defaults.text_transform);
        format->face_name = format_defaults.face_name;
        format->fontset = format_defaults.fontset;

        tree_->apply(format, feature, attrs, output);
    }
    else MAPNIK_LOG_WARN(text_properties) << "text_symbolizer_properties can't produce text: No formatting tree!";
}

void text_symbolizer_properties::set_format_tree(formatting::node_ptr tree)
{
    tree_ = tree;
}

formatting::node_ptr text_symbolizer_properties::format_tree() const
{
    return tree_;
}

void text_symbolizer_properties::text_properties_from_xml(xml_node const& node)
{
    // The options 'text-margin' and 'repeat-distance' replace 'minimum-distance'.
    // Only allow one or the other to be defined here.
    if (node.has_attribute("text-margin") || node.has_attribute("repeat-distance"))
    {
        if (node.has_attribute("minimum-distance"))
        {
            throw config_error(std::string("Cannot use deprecated option minimum-distance with "
                                           "new options text-margin and repeat-distance."));
        }
        set_property_from_xml<value_double>(expressions.text_margin, "text-margin", node);
        set_property_from_xml<value_double>(expressions.repeat_distance, "repeat-distance", node);
    }
    else
    {
        set_property_from_xml<value_double>(expressions.minimum_distance, "minimum-distance", node);
    }
    set_property_from_xml<label_placement_e>(expressions.label_placement, "placement", node);
    set_property_from_xml<value_double>(expressions.label_spacing, "spacing", node);
    set_property_from_xml<value_double>(expressions.label_position_tolerance, "label-position-tolerance", node);
    set_property_from_xml<value_double>(expressions.minimum_padding, "minimum-padding", node);
    set_property_from_xml<value_double>(expressions.minimum_path_length, "minimum-path-length", node);
    set_property_from_xml<boolean_type>(expressions.avoid_edges, "avoid-edges", node);
    set_property_from_xml<boolean_type>(expressions.allow_overlap, "allow-overlap", node);
    set_property_from_xml<boolean_type>(expressions.largest_bbox_only, "largest-bbox-only", node);
    set_property_from_xml<value_double>(expressions.max_char_angle_delta, "max-char-angle-delta", node);
    set_property_from_xml<text_upright_e>(expressions.upright, "upright", node);
}

void text_symbolizer_properties::from_xml(xml_node const& node, fontset_map const& fontsets)
{
    text_properties_from_xml(node);
    layout_defaults.from_xml(node,fontsets);
    optional<expression_ptr> name_ = node.get_opt_attr<expression_ptr>("name");
    if (name_)
    {
        MAPNIK_LOG_WARN(text_placements) << "Using 'name' in TextSymbolizer/ShieldSymbolizer is deprecated!";
        set_old_style_expression(*name_);
    }

    format_defaults.from_xml(node, fontsets);
    formatting::node_ptr n(formatting::node::from_xml(node,fontsets));
    if (n) set_format_tree(n);
}

void text_symbolizer_properties::to_xml(boost::property_tree::ptree &node,
                                        bool explicit_defaults,
                                        text_symbolizer_properties const& dfl) const
{
    if (!(expressions.label_placement == dfl.expressions.label_placement) || explicit_defaults)
    {
        serialize_property("placement", expressions.label_placement, node);
    }
    if (!(expressions.label_position_tolerance == dfl.expressions.label_position_tolerance) || explicit_defaults)
    {
        serialize_property("label-position-tolerance", expressions.label_position_tolerance,node);
    }
    if (!(expressions.label_spacing == dfl.expressions.label_spacing) || explicit_defaults)
    {
        serialize_property("spacing", expressions.label_spacing, node);
    }
    if (!(expressions.text_margin == dfl.expressions.text_margin) || explicit_defaults)
    {
        serialize_property("text-margin", expressions.text_margin, node);
    }
    if (!(expressions.repeat_distance == dfl.expressions.repeat_distance) || explicit_defaults)
    {
        serialize_property("repeat-distance", expressions.repeat_distance, node);
    }
    if (!(expressions.minimum_distance == dfl.expressions.minimum_distance) || explicit_defaults)
    {
        serialize_property("minimum-distance", expressions.minimum_distance, node);
    }
    if (!(expressions.minimum_padding == dfl.expressions.minimum_padding) || explicit_defaults)
    {
        serialize_property("minimum-padding", expressions.minimum_padding, node);
    }
    if (!(expressions.minimum_path_length == dfl.expressions.minimum_path_length) || explicit_defaults)
    {
        serialize_property("minimum-path-length", expressions.minimum_path_length, node);
    }
    if (!(expressions.avoid_edges == dfl.expressions.avoid_edges) || explicit_defaults)
    {
        serialize_property("avoid-edges", expressions.avoid_edges, node);
    }
    if (!(expressions.allow_overlap == dfl.expressions.allow_overlap) || explicit_defaults)
    {
        serialize_property("allow-overlap", expressions.allow_overlap, node);
    }
    if (!(expressions.largest_bbox_only == dfl.expressions.largest_bbox_only) || explicit_defaults)
    {
        serialize_property("largest-bbox-only", expressions.largest_bbox_only, node);
    }
    if (!(expressions.max_char_angle_delta == dfl.expressions.max_char_angle_delta) || explicit_defaults)
    {
        serialize_property("max-char-angle-delta", expressions.max_char_angle_delta, node);
    }
    if (!(expressions.upright == dfl.expressions.upright) || explicit_defaults)
    {
        serialize_property("upright", expressions.upright, node);
    }

    layout_defaults.to_xml(node, explicit_defaults, dfl.layout_defaults);
    format_defaults.to_xml(node, explicit_defaults, dfl.format_defaults);
    if (tree_) tree_->to_xml(node);
}


void text_symbolizer_properties::add_expressions(expression_set & output) const
{
    if (is_expression(expressions.label_placement)) output.insert(util::get<expression_ptr>(expressions.label_placement));
    if (is_expression(expressions.label_spacing)) output.insert(util::get<expression_ptr>(expressions.label_spacing));
    if (is_expression(expressions.label_position_tolerance)) output.insert(util::get<expression_ptr>(expressions.label_position_tolerance));
    if (is_expression(expressions.avoid_edges)) output.insert(util::get<expression_ptr>(expressions.avoid_edges));
    if (is_expression(expressions.text_margin)) output.insert(util::get<expression_ptr>(expressions.text_margin));
    if (is_expression(expressions.repeat_distance)) output.insert(util::get<expression_ptr>(expressions.repeat_distance));
    if (is_expression(expressions.minimum_distance)) output.insert(util::get<expression_ptr>(expressions.minimum_distance));
    if (is_expression(expressions.minimum_padding)) output.insert(util::get<expression_ptr>(expressions.minimum_padding));
    if (is_expression(expressions.minimum_path_length)) output.insert(util::get<expression_ptr>(expressions.minimum_path_length));
    if (is_expression(expressions.max_char_angle_delta)) output.insert(util::get<expression_ptr>(expressions.max_char_angle_delta));
    if (is_expression(expressions.allow_overlap)) output.insert(util::get<expression_ptr>(expressions.allow_overlap));
    if (is_expression(expressions.largest_bbox_only)) output.insert(util::get<expression_ptr>(expressions.largest_bbox_only));
    if (is_expression(expressions.upright)) output.insert(util::get<expression_ptr>(expressions.upright));

    layout_defaults.add_expressions(output);
    format_defaults.add_expressions(output);
    if (tree_) tree_->add_expressions(output);
}

void text_symbolizer_properties::set_old_style_expression(expression_ptr expr)
{
    tree_ = std::make_shared<formatting::text_node>(expr);
}

text_layout_properties::text_layout_properties()
    : halign(enumeration_wrapper(H_AUTO)),
      jalign(enumeration_wrapper(J_AUTO)),
      valign(enumeration_wrapper(V_AUTO)) {}

void text_layout_properties::from_xml(xml_node const &node, fontset_map const& fontsets)
{
    set_property_from_xml<double>(dx, "dx", node);
    set_property_from_xml<double>(dy, "dy", node);
    set_property_from_xml<double>(text_ratio, "text-ratio", node);
    set_property_from_xml<double>(wrap_width, "wrap-width", node);
    set_property_from_xml<std::string>(wrap_char, "wrap-char", node);
    set_property_from_xml<boolean_type>(wrap_before, "wrap-before", node);
    set_property_from_xml<boolean_type>(repeat_wrap_char, "repeat-wrap-char", node);
    set_property_from_xml<boolean_type>(rotate_displacement, "rotate-displacement", node);
    set_property_from_xml<double>(orientation, "orientation", node);
    set_property_from_xml<vertical_alignment_e>(valign, "vertical-alignment", node);
    set_property_from_xml<horizontal_alignment_e>(halign, "horizontal-alignment", node);
    set_property_from_xml<justify_alignment_e>(jalign, "justify-alignment", node);
}

void text_layout_properties::to_xml(boost::property_tree::ptree & node,
                                    bool explicit_defaults,
                                    text_layout_properties const& dfl) const
{
    if (!(dx == dfl.dx) || explicit_defaults) serialize_property("dx", dx, node);
    if (!(dy == dfl.dy) || explicit_defaults) serialize_property("dy", dy, node);
    if (!(valign == dfl.valign) || explicit_defaults) serialize_property("vertical-alignment", valign, node);
    if (!(halign == dfl.halign) || explicit_defaults) serialize_property("horizontal-alignment", halign, node);
    if (!(jalign == dfl.jalign) || explicit_defaults) serialize_property("justify-alignment", jalign, node);
    if (!(text_ratio == dfl.text_ratio) || explicit_defaults) serialize_property("text-ratio", text_ratio, node);
    if (!(wrap_width == dfl.wrap_width) || explicit_defaults) serialize_property("wrap-width", wrap_width, node);
    if (!(wrap_char == dfl.wrap_char) || explicit_defaults) serialize_property("wrap-char", wrap_char, node);
    if (!(wrap_before == dfl.wrap_before) || explicit_defaults) serialize_property("wrap-before", wrap_before, node);
    if (!(repeat_wrap_char == dfl.repeat_wrap_char) || explicit_defaults) serialize_property("repeat-wrap-char", repeat_wrap_char, node);
    if (!(rotate_displacement == dfl.rotate_displacement) || explicit_defaults)
        serialize_property("rotate-displacement", rotate_displacement, node);
    if (!(orientation == dfl.orientation) || explicit_defaults) serialize_property("orientation", orientation, node);
}

void text_layout_properties::add_expressions(expression_set & output) const
{
    if (is_expression(dx)) output.insert(util::get<expression_ptr>(dx));
    if (is_expression(dy)) output.insert(util::get<expression_ptr>(dy));
    if (is_expression(orientation)) output.insert(util::get<expression_ptr>(orientation));
    if (is_expression(wrap_width)) output.insert(util::get<expression_ptr>(wrap_width));
    if (is_expression(wrap_char)) output.insert(util::get<expression_ptr>(wrap_char));
    if (is_expression(repeat_wrap_char)) output.insert(util::get<expression_ptr>(repeat_wrap_char));
    if (is_expression(rotate_displacement)) output.insert(util::get<expression_ptr>(rotate_displacement));
    if (is_expression(text_ratio)) output.insert(util::get<expression_ptr>(text_ratio));
    if (is_expression(halign)) output.insert(util::get<expression_ptr>(halign));
    if (is_expression(valign)) output.insert(util::get<expression_ptr>(valign));
    if (is_expression(jalign)) output.insert(util::get<expression_ptr>(jalign));
}

// text format properties
format_properties::format_properties()
    : face_name(),
      fontset(),
      text_size(10.0),
      character_spacing(0.0),
      line_spacing(0.0),
      text_opacity(1.0),
      halo_opacity(1.0),
      fill(color(0,0,0)),
      halo_fill(color(255,255,255)),
      halo_radius(0.0),
      text_transform(enumeration_wrapper(NONE)) {}

void format_properties::from_xml(xml_node const& node, fontset_map const& fontsets)
{
    set_property_from_xml<double>(text_size, "size", node);
    set_property_from_xml<double>(character_spacing, "character-spacing", node);
    set_property_from_xml<double>(line_spacing, "line-spacing", node);
    set_property_from_xml<double>(halo_radius, "halo-radius", node);
    set_property_from_xml<double>(text_opacity, "opacity", node);
    set_property_from_xml<double>(halo_opacity, "halo-opacity", node);
    set_property_from_xml<color>(fill, "fill", node);
    set_property_from_xml<color>(halo_fill, "halo-fill", node);
    set_property_from_xml<text_transform_e>(text_transform,"text-transform", node);

    optional<std::string> face_name_ = node.get_opt_attr<std::string>("face-name");
    if (face_name_) face_name = *face_name_;
    optional<std::string> fontset_name_ = node.get_opt_attr<std::string>("fontset-name");
    if (fontset_name_)
    {
        std::map<std::string,font_set>::const_iterator itr = fontsets.find(*fontset_name_);
        if (itr != fontsets.end())
        {
            fontset = itr->second;
        }
        else
        {
            throw config_error("Unable to find any fontset named '" + *fontset_name_ + "'", node);
        }
    }
    if (!face_name.empty() && fontset)
    {
        throw config_error("Can't have both face-name and fontset-name", node);
    }
    if (face_name.empty() && !fontset)
    {
        throw config_error("Must have face-name or fontset-name", node);
    }
}

void format_properties::to_xml(boost::property_tree::ptree & node, bool explicit_defaults, format_properties const& dfl) const
{
    if (fontset)
    {
        set_attr(node, "fontset-name", fontset->get_name());
    }

    if (face_name != dfl.face_name || explicit_defaults)
    {
        set_attr(node, "face-name", face_name);
    }

    if (!(text_size == dfl.text_size) || explicit_defaults) serialize_property("size", text_size, node);
    if (!(character_spacing == dfl.character_spacing) || explicit_defaults) serialize_property("character-spacing", character_spacing, node);
    if (!(line_spacing == dfl.line_spacing) || explicit_defaults) serialize_property("line-spacing", line_spacing, node);
    if (!(halo_radius == dfl.halo_radius) || explicit_defaults) serialize_property("halo-radius", halo_radius, node);
    // for shield_symbolizer this is later overridden -- FIXME
    if (!(text_opacity == dfl.text_opacity) || explicit_defaults) serialize_property("opacity", text_opacity, node);
    if (!(halo_opacity == dfl.halo_opacity) || explicit_defaults) serialize_property("halo-opacity", halo_opacity, node);
    //
    if (!(fill == dfl.fill) || explicit_defaults) serialize_property("fill", fill, node);
    if (!(halo_fill == dfl.halo_fill) || explicit_defaults) serialize_property("halo-fill", halo_fill, node);
    if (!(text_transform == dfl.text_transform) || explicit_defaults) serialize_property("text-transform", text_transform, node);
}

void format_properties::add_expressions(expression_set & output) const
{
    if (is_expression(text_size)) output.insert(util::get<expression_ptr>(text_size));
    if (is_expression(character_spacing)) output.insert(util::get<expression_ptr>(character_spacing));
    if (is_expression(line_spacing)) output.insert(util::get<expression_ptr>(line_spacing));
    if (is_expression(halo_radius)) output.insert(util::get<expression_ptr>(halo_radius));
    if (is_expression(text_opacity)) output.insert(util::get<expression_ptr>(text_opacity));
    if (is_expression(halo_opacity)) output.insert(util::get<expression_ptr>(halo_opacity));
    if (is_expression(fill)) output.insert(util::get<expression_ptr>(fill));
    if (is_expression(halo_fill)) output.insert(util::get<expression_ptr>(halo_fill));
    if (is_expression(text_transform)) output.insert(util::get<expression_ptr>(text_transform));
}


} //ns mapnik
