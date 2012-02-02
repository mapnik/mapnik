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
#include <mapnik/text_processing.hpp>
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

void node::add_expressions(std::set<expression_ptr> &expressions) const
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


void list_node::add_expressions(std::set<expression_ptr> &expressions) const
{
    std::vector<node_ptr>::const_iterator itr = children_.begin();
    std::vector<node_ptr>::const_iterator end = children_.end();
    for (;itr != end; itr++)
    {
        (*itr)->add_expressions(expressions);
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


void text_node::add_expressions(std::set<expression_ptr> &expressions) const
{
    if (text_) expressions.insert(text_);
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

format_node::format_node():
    node(),
    fill_(),
    child_()
{

}

void format_node::to_xml(ptree &xml) const
{
    ptree &new_node = xml.push_back(ptree::value_type("Format", ptree()))->second;
    if (face_name_) set_attr(new_node, "face-name", *face_name_);
    if (text_size_) set_attr(new_node, "size", *text_size_);
    if (character_spacing_) set_attr(new_node, "character-spacing", *character_spacing_);
    if (line_spacing_) set_attr(new_node, "line-spacing", *line_spacing_);
    if (text_opacity_) set_attr(new_node, "opacity", *text_opacity_);
    if (wrap_before_) set_attr(new_node, "wrap-before", *wrap_before_);
    if (wrap_char_) set_attr(new_node, "wrap-character", *wrap_char_);
    if (text_transform_) set_attr(new_node, "text-transform", *text_transform_);
    if (fill_) set_attr(new_node, "fill", *fill_);
    if (halo_fill_) set_attr(new_node, "halo-fill", *halo_fill_);
    if (halo_radius_) set_attr(new_node, "halo-radius", *halo_radius_);
    if (child_) child_->to_xml(new_node);
}


node_ptr format_node::from_xml(ptree const& xml)
{
    format_node *n = new format_node();
    node_ptr np(n);

    node_ptr child = node::from_xml(xml);
    n->set_child(child);

    n->set_face_name(get_opt_attr<std::string>(xml, "face-name"));
    /*TODO: Fontset is problematic. We don't have the fontsets pointer here... */
    n->set_text_size(get_opt_attr<unsigned>(xml, "size"));
    n->set_character_spacing(get_opt_attr<unsigned>(xml, "character-spacing"));
    n->set_line_spacing(get_opt_attr<unsigned>(xml, "line-spacing"));
    n->set_text_opacity(get_opt_attr<double>(xml, "opactity"));
    boost::optional<boolean> wrap = get_opt_attr<boolean>(xml, "wrap-before");
    boost::optional<bool> wrap_before;
    if (wrap) wrap_before = *wrap;
    n->set_wrap_before(wrap_before);
    n->set_wrap_char(get_opt_attr<unsigned>(xml, "wrap-character"));
    n->set_text_transform(get_opt_attr<text_transform_e>(xml, "text-transform"));
    n->set_fill(get_opt_attr<color>(xml, "fill"));
    n->set_halo_fill(get_opt_attr<color>(xml, "halo-fill"));
    n->set_halo_radius(get_opt_attr<double>(xml, "halo-radius"));
    return np;
}


void format_node::apply(char_properties const& p, const Feature &feature, processed_text &output) const
{
    char_properties new_properties = p;
    if (face_name_) new_properties.face_name = *face_name_;
    if (text_size_) new_properties.text_size = *text_size_;
    if (character_spacing_) new_properties.character_spacing = *character_spacing_;
    if (line_spacing_) new_properties.line_spacing = *line_spacing_;
    if (text_opacity_) new_properties.text_opacity = *text_opacity_;
    if (wrap_before_) new_properties.wrap_before = *wrap_before_;
    if (wrap_char_) new_properties.wrap_char = *wrap_char_;
    if (text_transform_) new_properties.text_transform = *text_transform_;
    if (fill_) new_properties.fill = *fill_;
    if (halo_fill_) new_properties.halo_fill = *halo_fill_;
    if (halo_radius_) new_properties.halo_radius = *halo_radius_;

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


void format_node::set_face_name(optional<std::string> face_name)
{
    face_name_ = face_name;
}

void format_node::set_text_size(optional<unsigned> text_size)
{
    text_size_ = text_size;
}

void format_node::set_character_spacing(optional<unsigned> character_spacing)
{
    character_spacing_ = character_spacing;
}

void format_node::set_line_spacing(optional<unsigned> line_spacing)
{
    line_spacing_ = line_spacing;
}

void format_node::set_text_opacity(optional<double> text_opacity)
{
    text_opacity_ = text_opacity;
}

void format_node::set_wrap_before(optional<bool> wrap_before)
{
    wrap_before_ = wrap_before;
}

void format_node::set_wrap_char(optional<unsigned> wrap_char)
{
    wrap_char_ = wrap_char;
}

void format_node::set_text_transform(optional<text_transform_e> text_transform)
{
    text_transform_ = text_transform;
}

void format_node::set_fill(optional<color> c)
{
    fill_ = c;
}

void format_node::set_halo_fill(optional<color> c)
{
    halo_fill_ = c;
}

void format_node::set_halo_radius(optional<double> radius)
{
    halo_radius_ = radius;
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
