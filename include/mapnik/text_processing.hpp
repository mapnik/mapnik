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
#ifndef MAPNIK_TEXT_PROCESSING_HPP
#define MAPNIK_TEXT_PROCESSING_HPP

#include <boost/property_tree/ptree.hpp>
#include <boost/optional.hpp>

#include <mapnik/feature.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text_path.hpp>
#include <mapnik/enumeration.hpp>
#include <mapnik/filter_factory.hpp>

#include <list>
#include <vector>
#include <set>
namespace mapnik
{
class processed_text;
struct char_properties;



enum label_placement_enum {
    POINT_PLACEMENT,
    LINE_PLACEMENT,
    VERTEX_PLACEMENT,
    INTERIOR_PLACEMENT,
    label_placement_enum_MAX
};

DEFINE_ENUM( label_placement_e, label_placement_enum );

enum vertical_alignment
{
    V_TOP = 0,
    V_MIDDLE,
    V_BOTTOM,
    V_AUTO,
    vertical_alignment_MAX
};

DEFINE_ENUM( vertical_alignment_e, vertical_alignment );

enum horizontal_alignment
{
    H_LEFT = 0,
    H_MIDDLE,
    H_RIGHT,
    H_AUTO,
    horizontal_alignment_MAX
};

DEFINE_ENUM( horizontal_alignment_e, horizontal_alignment );

enum justify_alignment
{
    J_LEFT = 0,
    J_MIDDLE,
    J_RIGHT,
    justify_alignment_MAX
};

DEFINE_ENUM( justify_alignment_e, justify_alignment );

enum text_transform
{
    NONE = 0,
    UPPERCASE,
    LOWERCASE,
    CAPITALIZE,
    text_transform_MAX
};
DEFINE_ENUM( text_transform_e, text_transform );

namespace formating {

class list_node: public node {
public:
    list_node() : node(), children_() {}
    virtual void to_xml(boost::property_tree::ptree &xml) const;
    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const;
    virtual void add_expressions(expression_set &output) const;

    void push_back(node_ptr n);
    void set_children(std::vector<node_ptr> const& children);
    std::vector<node_ptr> const& get_children() const;
    void clear();
private:
    std::vector<node_ptr> children_;
};

class text_node: public node {
public:
    text_node(expression_ptr text): node(), text_(text) {}
    text_node(std::string text): node(), text_(parse_expression(text)) {}
    void to_xml(boost::property_tree::ptree &xml) const;
    static node_ptr from_xml(boost::property_tree::ptree const& xml);
    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const;
    virtual void add_expressions(expression_set &output) const;

    void set_text(expression_ptr text);
    expression_ptr get_text() const;
private:
    expression_ptr text_;
};

class format_node: public node {
public:
    void to_xml(boost::property_tree::ptree &xml) const;
    static node_ptr from_xml(boost::property_tree::ptree const& xml);
    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const;
    virtual void add_expressions(expression_set &output) const;

    void set_child(node_ptr child);
    node_ptr get_child() const;

    boost::optional<std::string> face_name;
    boost::optional<unsigned> text_size;
    boost::optional<unsigned> character_spacing;
    boost::optional<unsigned> line_spacing;
    boost::optional<double> text_opacity;
    boost::optional<bool> wrap_before;
    boost::optional<unsigned> wrap_char;
    boost::optional<text_transform_e> text_transform;
    boost::optional<color> fill;
    boost::optional<color> halo_fill;
    boost::optional<double> halo_radius;

private:
    node_ptr child_;
};

} //namespace formating

} /* namespace mapnik*/

#endif
