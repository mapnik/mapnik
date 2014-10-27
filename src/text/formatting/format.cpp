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
#include <mapnik/feature.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/formatting/format.hpp>
#include <mapnik/text/properties_util.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/xml_node.hpp>

//boost
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>

namespace mapnik { namespace formatting {

using boost::property_tree::ptree;

void format_node::to_xml(ptree & xml) const
{
    ptree & new_node = xml.push_back(ptree::value_type("Format", ptree()))->second;

    if (text_size) serialize_property("size", *text_size, new_node);
    if (character_spacing) serialize_property("character-spacing", *character_spacing, new_node);
    if (line_spacing) serialize_property("line-spacing", *line_spacing, new_node);
    if (text_opacity) serialize_property("opacity", *text_opacity, new_node);
    if (wrap_before) serialize_property("wrap-before", *wrap_before, new_node);
    if (repeat_wrap_char) serialize_property("repeat-wrap-character", *repeat_wrap_char, new_node);
    if (fill) serialize_property("fill", *fill, new_node);
    if (halo_fill) serialize_property("halo-fill", *halo_fill, new_node);
    if (halo_radius) serialize_property("halo-radius", *halo_radius, new_node);
    if (text_transform) serialize_property("text-transform", *text_transform, new_node);
    if (ff_settings) serialize_property("font-feature-settings", *ff_settings, new_node);

    if (face_name) set_attr(new_node, "face-name", *face_name);
    if (fontset) set_attr(new_node, "fontset-name", fontset->get_name());

    if (child_) child_->to_xml(new_node);
}


node_ptr format_node::from_xml(xml_node const& xml, fontset_map const& fontsets)
{
    auto n = std::make_shared<format_node>();
    node_ptr child = node::from_xml(xml,fontsets);
    n->set_child(child);

    set_property_from_xml<double>(n->text_size, "size", xml);
    set_property_from_xml<double>(n->character_spacing, "character-spacing", xml);
    set_property_from_xml<double>(n->line_spacing, "line-spacing", xml);
    set_property_from_xml<double>(n->text_opacity, "opacity", xml);
    //set_property_from_xml<double>(n->halo_opacity, "halo-opacity", xml); FIXME
    set_property_from_xml<double>(n->halo_radius, "halo-radius", xml);
    set_property_from_xml<color>(n->fill, "fill", xml);
    set_property_from_xml<color>(n->halo_fill, "halo-fill", xml);
    set_property_from_xml<text_transform_e>(n->text_transform, "text-transform", xml);
    set_property_from_xml<font_feature_settings>(n->ff_settings, "font-feature-settings", xml);

    boost::optional<std::string> face_name = xml.get_opt_attr<std::string>("face-name");
    if (face_name)
    {
        n->face_name = *face_name;
    }
    boost::optional<std::string> fontset_name = xml.get_opt_attr<std::string>("fontset-name");
    if (fontset_name)
    {
        std::map<std::string,font_set>::const_iterator itr = fontsets.find(*fontset_name);
        if (itr != fontsets.end())
        {
            n->fontset = itr->second;
        }
        else
        {
            throw config_error("Unable to find any fontset named '" + *fontset_name + "' for <Format> node", xml);
        }
    }
    if (face_name && !face_name->empty() && n->fontset)
    {
        throw config_error("Can't have both face-name and fontset-name", xml);
    }
    return n;
}


void format_node::apply(evaluated_format_properties_ptr const& p, feature_impl const& feature, attributes const& attrs, text_layout &output) const
{

    evaluated_format_properties_ptr & new_properties = output.new_child_format_ptr(p);
    if (text_size) new_properties->text_size = util::apply_visitor(extract_value<value_double>(feature,attrs), *text_size);
    if (character_spacing) new_properties->character_spacing = util::apply_visitor(extract_value<value_double>(feature,attrs), *character_spacing);
    if (line_spacing) new_properties->line_spacing = util::apply_visitor(extract_value<value_double>(feature,attrs), *line_spacing);
    if (text_opacity) new_properties->text_opacity = util::apply_visitor(extract_value<value_double>(feature,attrs), *text_opacity);
    if (halo_radius) new_properties->halo_radius = util::apply_visitor(extract_value<value_double>(feature,attrs), *halo_radius);
    if (fill) new_properties->fill = util::apply_visitor(extract_value<color>(feature,attrs), *fill);
    if (halo_fill) new_properties->halo_fill = util::apply_visitor(extract_value<color>(feature,attrs), *halo_fill);
    if (text_transform) new_properties->text_transform = util::apply_visitor(extract_value<text_transform_enum>(feature,attrs), *text_transform);
    if (ff_settings) new_properties->ff_settings = util::apply_visitor(extract_value<font_feature_settings>(feature,attrs), *ff_settings);

    if (fontset)
    {
        new_properties->fontset = *fontset;
    }
    else
    {
        if (face_name)
        {
            new_properties->face_name = *face_name;
            new_properties->fontset.reset();
        }
    }

    if (child_) child_->apply(new_properties, feature, attrs, output);
    else MAPNIK_LOG_WARN(format) << "Useless format: No text to format";
}


void format_node::set_child(node_ptr child)
{
    child_ = child;
}


node_ptr format_node::get_child() const
{
    return child_;
}

void format_node::add_expressions(expression_set & output) const
{
    if (text_size && is_expression(*text_size)) output.insert(util::get<expression_ptr>(*text_size));
    if (character_spacing && is_expression(*character_spacing)) output.insert(util::get<expression_ptr>(*character_spacing));
    if (line_spacing && is_expression(*line_spacing)) output.insert(util::get<expression_ptr>(*line_spacing));
    if (halo_radius && is_expression(*halo_radius)) output.insert(util::get<expression_ptr>(*halo_radius));
    if (text_opacity && is_expression(*text_opacity)) output.insert(util::get<expression_ptr>(*text_opacity));
    //if (halo_opacity && is_expression(*halo_opacity)) output.insert(util::get<expression_ptr>(*halo_opacity));
    if (wrap_before && is_expression(*wrap_before)) output.insert(util::get<expression_ptr>(*wrap_before));
    if (repeat_wrap_char && is_expression(*repeat_wrap_char)) output.insert(util::get<expression_ptr>(*repeat_wrap_char));
    if (fill && is_expression(*fill)) output.insert(util::get<expression_ptr>(*fill));
    if (halo_fill && is_expression(*halo_fill)) output.insert(util::get<expression_ptr>(*halo_fill));
    if (text_transform && is_expression(*text_transform)) output.insert(util::get<expression_ptr>(*text_transform));
    if (ff_settings && is_expression(*ff_settings)) output.insert(util::get<expression_ptr>(*ff_settings));
    if (child_) child_->add_expressions(output);
}

} //ns formatting
} //ns mapnik
