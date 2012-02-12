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
#include <mapnik/text_placements.hpp>
#include <mapnik/color.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/ptree_helpers.hpp>

#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>

#include <stack>
#include <vector>

namespace mapnik {
using boost::property_tree::ptree;
using boost::optional;

namespace formating {

void node::to_xml(boost::property_tree::ptree &xml) const
{
    //TODO: Should this throw a config_error?
#ifdef MAPNIK_DEBUG
    std::cerr << "Error: Trying to write unsupported node type to XML.\n";
#endif
}

node_ptr node::from_xml(boost::property_tree::ptree const& xml)
{
    list_node *list = new list_node();
    node_ptr list_ptr(list);
    ptree::const_iterator itr = xml.begin();
    ptree::const_iterator end = xml.end();
    for (; itr != end; ++itr) {
        node_ptr n;
        if (itr->first == "<xmltext>") {
            n = text_node::from_xml(itr->second);
        } else if (itr->first == "Format") {
            n = format_node::from_xml(itr->second);
        } else if (itr->first != "<xmlcomment>" && itr->first != "<xmlattr>" && itr->first != "Placement") {
            throw config_error("Unknown item " + itr->first);
        }
        if (n) list->push_back(n);
    }
    if (list->get_children().size() == 1) {
        return list->get_children()[0];
    } else if (list->get_children().size() > 1) {
        return list_ptr;
    } else {
        return node_ptr();
    }
}

void node::add_expressions(expression_set &output) const
{
    //Do nothing by default
}

/************************************************************/

void list_node::to_xml(boost::property_tree::ptree &xml) const
{
    std::vector<node_ptr>::const_iterator itr = children_.begin();
    std::vector<node_ptr>::const_iterator end = children_.end();
    for (;itr != end; itr++)
    {
        (*itr)->to_xml(xml);
    }
}


void list_node::apply(char_properties const& p, Feature const& feature, processed_text &output) const
{
    std::vector<node_ptr>::const_iterator itr = children_.begin();
    std::vector<node_ptr>::const_iterator end = children_.end();
    for (;itr != end; itr++)
    {
        (*itr)->apply(p, feature, output);
    }
}


void list_node::add_expressions(expression_set &output) const
{
    std::vector<node_ptr>::const_iterator itr = children_.begin();
    std::vector<node_ptr>::const_iterator end = children_.end();
    for (;itr != end; itr++)
    {
        (*itr)->add_expressions(output);
    }
}


void list_node::push_back(node_ptr n)
{
    children_.push_back(n);
}


void list_node::clear()
{
    children_.clear();
}

void list_node::set_children(std::vector<node_ptr> const& children)
{
    children_ = children;
}

std::vector<node_ptr> const& list_node::get_children() const
{
    return children_;
}

/************************************************************/

void text_node::to_xml(ptree &xml) const
{
    ptree &new_node = xml.push_back(ptree::value_type(
                                        "<xmltext>", ptree()))->second;
    new_node.put_value(to_expression_string(*text_));
}


node_ptr text_node::from_xml(boost::property_tree::ptree const& xml)
{
    std::string data = xml.data();
    boost::trim(data);
    if (data.empty()) return node_ptr(); //No text
    return node_ptr(new text_node(parse_expression(data, "utf8")));
}

void text_node::apply(char_properties const& p, Feature const& feature, processed_text &output) const
{
    UnicodeString text_str = boost::apply_visitor(evaluate<Feature,value_type>(feature), *text_).to_unicode();
    if (p.text_transform == UPPERCASE)
    {
        text_str = text_str.toUpper();
    }
    else if (p.text_transform == LOWERCASE)
    {
        text_str = text_str.toLower();
    }
    else if (p.text_transform == CAPITALIZE)
    {
        text_str = text_str.toTitle(NULL);
    }
    if (text_str.length() > 0) {
        output.push_back(p, text_str);
    } else {
#ifdef MAPNIK_DEBUG
        std::cerr << "Warning: Empty expression.\n";
#endif
    }
}


void text_node::add_expressions(expression_set &output) const
{
    if (text_) output.insert(text_);
}


void text_node::set_text(expression_ptr text)
{
    text_ = text;
}


expression_ptr text_node::get_text() const
{
    return text_;
}

/************************************************************/

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


node_ptr format_node::from_xml(ptree const& xml)
{
    format_node *n = new format_node();
    node_ptr np(n);

    node_ptr child = node::from_xml(xml);
    n->set_child(child);

    n->face_name = get_opt_attr<std::string>(xml, "face-name");
    /*TODO: Fontset is problematic. We don't have the fontsets pointer here... */
    n->text_size = get_opt_attr<unsigned>(xml, "size");
    n->character_spacing = get_opt_attr<unsigned>(xml, "character-spacing");
    n->line_spacing = get_opt_attr<unsigned>(xml, "line-spacing");
    n->text_opacity = get_opt_attr<double>(xml, "opactity");
    boost::optional<boolean> wrap = get_opt_attr<boolean>(xml, "wrap-before");
    if (wrap) n->wrap_before = *wrap;
    n->wrap_char = get_opt_attr<unsigned>(xml, "wrap-character");
    n->text_transform = get_opt_attr<text_transform_e>(xml, "text-transform");
    n->fill = get_opt_attr<color>(xml, "fill");
    n->halo_fill = get_opt_attr<color>(xml, "halo-fill");
    n->halo_radius = get_opt_attr<double>(xml, "halo-radius");
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
#ifdef MAPNIK_DEBUG
        std::cerr << "Warning: Useless format: No text to format\n";
#endif
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

} //namespace formating

/************************************************************/

void processed_text::push_back(char_properties const& properties, UnicodeString const& text)
{
    expr_list_.push_back(processed_expression(properties, text));
}

processed_text::expression_list::const_iterator processed_text::begin() const
{
    return expr_list_.begin();
}

processed_text::expression_list::const_iterator processed_text::end() const
{
    return expr_list_.end();
}

processed_text::processed_text(face_manager<freetype_engine> & font_manager, double scale_factor)
    : font_manager_(font_manager), scale_factor_(scale_factor)
{

}

void processed_text::clear()
{
    info_.clear();
    expr_list_.clear();
}


string_info &processed_text::get_string_info()
{
    info_.clear(); //if this function is called twice invalid results are returned, so clear string_info first
    expression_list::iterator itr = expr_list_.begin();
    expression_list::iterator end = expr_list_.end();
    for (; itr != end; ++itr)
    {
        char_properties const &p = itr->p;
        face_set_ptr faces = font_manager_.get_face_set(p.face_name, p.fontset);
        if (faces->size() <= 0)
        {
            throw config_error("Unable to find specified font face '" + p.face_name + "'");
        }
        faces->set_character_sizes(p.text_size * scale_factor_);
        faces->get_string_info(info_, itr->str, &(itr->p));
        info_.add_text(itr->str);
    }
    return info_;
}



} /* namespace */
