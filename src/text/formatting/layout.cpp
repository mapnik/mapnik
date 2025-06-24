/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#include <mapnik/text/glyph_info.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/text/formatting/layout.hpp>
#include <mapnik/xml_node.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/text/properties_util.hpp>
#include <mapnik/boolean.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/property_tree/ptree.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik {
namespace formatting {

using boost::property_tree::ptree;

void layout_node::to_xml(ptree& xml) const
{
    ptree& new_node = xml.push_back(ptree::value_type("Layout", ptree()))->second;

    if (dx)
        serialize_property("dx", *dx, new_node);
    if (dy)
        serialize_property("dy", *dy, new_node);
    if (text_ratio)
        serialize_property("text-ratio", *text_ratio, new_node);
    if (wrap_width)
        serialize_property("wrap-width", *wrap_width, new_node);
    if (wrap_char)
        serialize_property("wrap-character", *wrap_char, new_node);
    if (wrap_before)
        serialize_property("wrap-before", *wrap_before, new_node);
    if (repeat_wrap_char)
        serialize_property("repeat-wrap-character", *repeat_wrap_char, new_node);
    if (rotate_displacement)
        serialize_property("rotate-displacement", *rotate_displacement, new_node);
    if (orientation)
        serialize_property("orientation", *orientation, new_node);
    if (halign)
        serialize_property("horizontal-alignment", *halign, new_node);
    if (valign)
        serialize_property("vertical-alignment", *valign, new_node);
    if (jalign)
        serialize_property("justify-alignment", *jalign, new_node);

    if (child_)
        child_->to_xml(new_node);
}

node_ptr layout_node::from_xml(xml_node const& xml, fontset_map const& fontsets)
{
    auto n = std::make_shared<layout_node>();
    node_ptr child = node::from_xml(xml, fontsets);
    n->set_child(child);

    if (xml.has_attribute("dx"))
        set_property_from_xml<double>(n->dx, "dx", xml);
    if (xml.has_attribute("dy"))
        set_property_from_xml<double>(n->dy, "dy", xml);
    if (xml.has_attribute("text-ratio"))
        set_property_from_xml<double>(n->text_ratio, "text-ratio", xml);
    if (xml.has_attribute("wrap-width"))
        set_property_from_xml<double>(n->wrap_width, "wrap-width", xml);
    if (xml.has_attribute("wrap-character"))
        set_property_from_xml<std::string>(n->wrap_char, "wrap-character", xml);
    if (xml.has_attribute("wrap-before"))
        set_property_from_xml<mapnik::value_bool>(n->wrap_before, "wrap-before", xml);
    if (xml.has_attribute("repeat-wrap-character"))
        set_property_from_xml<mapnik::value_bool>(n->repeat_wrap_char, "repeat-wrap-character", xml);
    if (xml.has_attribute("rotate-displacement"))
        set_property_from_xml<mapnik::value_bool>(n->rotate_displacement, "rotate-displacement", xml);
    if (xml.has_attribute("orientation"))
        set_property_from_xml<double>(n->orientation, "orientation", xml);
    if (xml.has_attribute("horizontal-alignment"))
        set_property_from_xml<horizontal_alignment_e>(n->halign, "horizontal-alignment", xml);
    if (xml.has_attribute("vertical-alignment"))
        set_property_from_xml<vertical_alignment_e>(n->valign, "vertical-alignment", xml);
    if (xml.has_attribute("justify-alignment"))
        set_property_from_xml<justify_alignment_e>(n->jalign, "justify-alignment", xml);
    return n;
}

void layout_node::apply(evaluated_format_properties_ptr const& p,
                        feature_impl const& feature,
                        attributes const& vars,
                        text_layout& parent) const
{
    text_layout_properties new_properties(parent.get_layout_properties());
    if (dx)
        new_properties.dx = *dx;
    if (dy)
        new_properties.dy = *dy;
    if (text_ratio)
        new_properties.text_ratio = *text_ratio;
    if (wrap_width)
        new_properties.wrap_width = *wrap_width;
    if (wrap_char)
        new_properties.wrap_char = *wrap_char;
    if (wrap_before)
        new_properties.wrap_before = *wrap_before;
    if (repeat_wrap_char)
        new_properties.repeat_wrap_char = *repeat_wrap_char;
    if (rotate_displacement)
        new_properties.rotate_displacement = *rotate_displacement;
    if (orientation)
        new_properties.orientation = *orientation;
    if (halign)
        new_properties.halign = *halign;
    if (valign)
        new_properties.valign = *valign;
    if (jalign)
        new_properties.jalign = *jalign;

    // starting a new offset child with the new displacement value
    // we pass a null format tree since this is not the parent but a child
    text_layout_ptr child_layout = std::make_shared<text_layout>(parent.get_font_manager(),
                                                                 feature,
                                                                 vars,
                                                                 parent.get_scale_factor(),
                                                                 parent.get_default_text_properties(),
                                                                 new_properties,
                                                                 formatting::node_ptr());
    // process contained format tree into the child node
    if (child_)
    {
        child_->apply(p, feature, vars, *child_layout);
    }
    else
    {
        MAPNIK_LOG_WARN(format) << "Useless layout node: Contains no text";
    }
    parent.add_child(child_layout);
}

void layout_node::set_child(node_ptr child)
{
    child_ = child;
}

node_ptr layout_node::get_child() const
{
    return child_;
}

void layout_node::add_expressions(expression_set& output) const
{
    if (dx && is_expression(*dx))
        output.insert(util::get<expression_ptr>(*dx));
    if (dy && is_expression(*dy))
        output.insert(util::get<expression_ptr>(*dy));
    if (orientation && is_expression(*orientation))
        output.insert(util::get<expression_ptr>(*orientation));
    if (wrap_width && is_expression(*wrap_width))
        output.insert(util::get<expression_ptr>(*wrap_width));
    if (wrap_char && is_expression(*wrap_char))
        output.insert(util::get<expression_ptr>(*wrap_char));
    if (wrap_before && is_expression(*wrap_before))
        output.insert(util::get<expression_ptr>(*wrap_before));
    if (repeat_wrap_char && is_expression(*repeat_wrap_char))
        output.insert(util::get<expression_ptr>(*repeat_wrap_char));
    if (rotate_displacement && is_expression(*rotate_displacement))
        output.insert(util::get<expression_ptr>(*rotate_displacement));
    if (text_ratio && is_expression(*text_ratio))
        output.insert(util::get<expression_ptr>(*text_ratio));
    if (halign && is_expression(*halign))
        output.insert(util::get<expression_ptr>(*halign));
    if (valign && is_expression(*valign))
        output.insert(util::get<expression_ptr>(*valign));
    if (jalign && is_expression(*jalign))
        output.insert(util::get<expression_ptr>(*jalign));

    if (child_)
        child_->add_expressions(output);
}

} // namespace formatting
} // namespace mapnik
