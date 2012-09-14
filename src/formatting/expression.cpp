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
#include <mapnik/formatting/expression_format.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/text_properties.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/xml_node.hpp>


namespace mapnik {
namespace formatting {

using boost::property_tree::ptree;
void expression_format::to_xml(boost::property_tree::ptree &xml) const
{
    ptree &new_node = xml.push_back(ptree::value_type("ExpressionFormat", ptree()))->second;
    if (face_name) set_attr(new_node, "face-name", to_expression_string(*face_name));
    if (text_size) set_attr(new_node, "size", to_expression_string(*text_size));
    if (character_spacing) set_attr(new_node, "character-spacing", to_expression_string(*character_spacing));
    if (line_spacing) set_attr(new_node, "line-spacing", to_expression_string(*line_spacing));
    if (text_opacity) set_attr(new_node, "opacity", to_expression_string(*text_opacity));
    if (wrap_before) set_attr(new_node, "wrap-before", to_expression_string(*wrap_before));
    if (wrap_char) set_attr(new_node, "wrap-character", to_expression_string(*wrap_char));
    if (fill) set_attr(new_node, "fill", to_expression_string(*fill));
    if (halo_fill) set_attr(new_node, "halo-fill", to_expression_string(*halo_fill));
    if (halo_radius) set_attr(new_node, "halo-radius", to_expression_string(*halo_radius));
    if (child_) child_->to_xml(new_node);
}

node_ptr expression_format::from_xml(xml_node const& xml)
{
    expression_format *n = new expression_format();
    node_ptr np(n);

    node_ptr child = node::from_xml(xml);
    n->set_child(child);

    n->face_name = get_expression(xml, "face-name");
    n->text_size = get_expression(xml, "size");
    n->character_spacing = get_expression(xml, "character-spacing");
    n->line_spacing = get_expression(xml, "line-spacing");
    n->text_opacity = get_expression(xml, "opacity");
    n->wrap_before = get_expression(xml, "wrap-before");
    n->wrap_char = get_expression(xml, "wrap-character");
    n->fill = get_expression(xml, "fill");
    n->halo_fill = get_expression(xml, "halo-fill");
    n->halo_radius = get_expression(xml, "halo-radius");
    return np;
}

expression_ptr expression_format::get_expression(xml_node const& xml, std::string name)
{
    boost::optional<expression_ptr> tmp = xml.get_opt_attr<expression_ptr>(name);
    if (tmp) return *tmp;
    return expression_ptr();
}


void expression_format::apply(char_properties const& p, const Feature &feature, processed_text &output) const
{
    char_properties new_properties = p;
    if (face_name) new_properties.face_name =
                       boost::apply_visitor(evaluate<Feature,value_type>(feature), *face_name).to_string();
    if (text_size) new_properties.text_size =
                       boost::apply_visitor(evaluate<Feature,value_type>(feature), *text_size).to_double();
    if (character_spacing) new_properties.character_spacing =
                               boost::apply_visitor(evaluate<Feature,value_type>(feature), *character_spacing).to_double();
    if (line_spacing) new_properties.line_spacing =
                          boost::apply_visitor(evaluate<Feature,value_type>(feature), *line_spacing).to_double();
    if (text_opacity) new_properties.text_opacity =
                          boost::apply_visitor(evaluate<Feature,value_type>(feature), *text_opacity).to_double();
    if (wrap_before) new_properties.wrap_before =
                         boost::apply_visitor(evaluate<Feature,value_type>(feature), *wrap_before).to_bool();
    if (wrap_char) new_properties.wrap_char =
                       boost::apply_visitor(evaluate<Feature,value_type>(feature), *character_spacing).to_unicode()[0];
//    if (fill) new_properties.fill =
//            boost::apply_visitor(evaluate<Feature,value_type>(feature), *fill).to_color();
//    if (halo_fill) new_properties.halo_fill =
//            boost::apply_visitor(evaluate<Feature,value_type>(feature), *halo_fill).to_color();
    if (halo_radius) new_properties.halo_radius =
                         boost::apply_visitor(evaluate<Feature,value_type>(feature), *halo_radius).to_double();

    if (child_) {
        child_->apply(new_properties, feature, output);
    } else {
        MAPNIK_LOG_WARN(expression) << "Useless format: No text to format";
    }
}


void expression_format::set_child(node_ptr child)
{
    child_ = child;
}


node_ptr expression_format::get_child() const
{
    return child_;
}

void expression_format::add_expressions(expression_set &output) const
{
    if (child_) child_->add_expressions(output);
    output.insert(face_name);
    output.insert(text_size);
    output.insert(character_spacing);
    output.insert(line_spacing);
    output.insert(text_opacity);
    output.insert(wrap_before);
    output.insert(wrap_char);
    output.insert(fill);
    output.insert(halo_fill);
    output.insert(halo_radius);
}

} //ns formatting
} //ns mapnik
