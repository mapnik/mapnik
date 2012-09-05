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

// mapnik
#include <mapnik/debug.hpp>
#include <mapnik/formatting/format.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/xml_node.hpp>

namespace mapnik {
namespace formatting {

using boost::property_tree::ptree;

void format_node::to_xml(ptree &xml) const
{
    ptree &new_node = xml.push_back(ptree::value_type("Format", ptree()))->second;
    if (face_name) set_attr(new_node, "face-name", *face_name);
    if (text_size) set_attr(new_node, "size", *text_size);
    if (character_spacing) set_attr(new_node, "character-spacing", *character_spacing);
    if (line_spacing) set_attr(new_node, "line-spacing", *line_spacing);
    if (text_opacity) set_attr(new_node, "opacity", *text_opacity);
    if (wrap_before) set_attr(new_node, "wrap-before", *wrap_before);
    if (wrap_char) set_attr(new_node, "wrap-character", *wrap_char);
    if (text_transform) set_attr(new_node, "text-transform", *text_transform);
    if (fill) set_attr(new_node, "fill", *fill);
    if (halo_fill) set_attr(new_node, "halo-fill", *halo_fill);
    if (halo_radius) set_attr(new_node, "halo-radius", *halo_radius);
    if (child_) child_->to_xml(new_node);
}


node_ptr format_node::from_xml(xml_node const& xml)
{
    format_node *n = new format_node();
    node_ptr np(n);

    node_ptr child = node::from_xml(xml);
    n->set_child(child);

    n->face_name = xml.get_opt_attr<std::string>("face-name");
    /*TODO: Fontset is problematic. We don't have the fontsets pointer here... */
    n->text_size = xml.get_opt_attr<double>("size");
    n->character_spacing = xml.get_opt_attr<double>("character-spacing");
    n->line_spacing = xml.get_opt_attr<double>("line-spacing");
    n->text_opacity = xml.get_opt_attr<double>("opacity");
    boost::optional<boolean> wrap = xml.get_opt_attr<boolean>("wrap-before");
    if (wrap) n->wrap_before = *wrap;
    n->wrap_char = xml.get_opt_attr<unsigned>("wrap-character");
    n->text_transform = xml.get_opt_attr<text_transform_e>("text-transform");
    n->fill = xml.get_opt_attr<color>("fill");
    n->halo_fill = xml.get_opt_attr<color>("halo-fill");
    n->halo_radius = xml.get_opt_attr<double>("halo-radius");
    return np;
}


void format_node::apply(char_properties const& p, const Feature &feature, processed_text &output) const
{
    char_properties new_properties = p;
    if (face_name) new_properties.face_name = *face_name;
    if (text_size) new_properties.text_size = *text_size;
    if (character_spacing) new_properties.character_spacing = *character_spacing;
    if (line_spacing) new_properties.line_spacing = *line_spacing;
    if (text_opacity) new_properties.text_opacity = *text_opacity;
    if (wrap_before) new_properties.wrap_before = *wrap_before;
    if (wrap_char) new_properties.wrap_char = *wrap_char;
    if (text_transform) new_properties.text_transform = *text_transform;
    if (fill) new_properties.fill = *fill;
    if (halo_fill) new_properties.halo_fill = *halo_fill;
    if (halo_radius) new_properties.halo_radius = *halo_radius;

    if (child_) {
        child_->apply(new_properties, feature, output);
    } else {
        MAPNIK_LOG_WARN(format) << "Useless format: No text to format";
    }
}


void format_node::set_child(node_ptr child)
{
    child_ = child;
}


node_ptr format_node::get_child() const
{
    return child_;
}

void format_node::add_expressions(expression_set &output) const
{
    if (child_) child_->add_expressions(output);
}

} //ns formatting
} //ns mapnik
