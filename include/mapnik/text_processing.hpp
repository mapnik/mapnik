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

enum text_transform
{
    NONE = 0,
    UPPERCASE,
    LOWERCASE,
    CAPITALIZE,
    text_transform_MAX
};
DEFINE_ENUM( text_transform_e, text_transform );


struct char_properties
{
    char_properties();
    /** Construct object from XML. */
    void set_values_from_xml(boost::property_tree::ptree const &sym, std::map<std::string,font_set> const & fontsets);
    /** Write object to XML ptree. */
    void to_xml(boost::property_tree::ptree &node, bool explicit_defaults, char_properties const &dfl=char_properties()) const;
    std::string face_name;
    font_set fontset;
    float text_size;
    double character_spacing;
    double line_spacing; //Largest total height (fontsize+line_spacing) per line is chosen
    double text_opacity;
    bool wrap_before;
    unsigned wrap_char;
    text_transform_e text_transform; //Per expression
    color fill;
    color halo_fill;
    double halo_radius;
};

class processed_expression
{
public:
    processed_expression(char_properties const& properties, UnicodeString const& text) :
        p(properties), str(text) {}
    char_properties p;
    UnicodeString str;
};


class processed_text : boost::noncopyable
{
public:
    processed_text(face_manager<freetype_engine> & font_manager, double scale_factor);
    void push_back(processed_expression const& exp);
    unsigned size() const { return expr_list_.size(); }
    unsigned empty() const { return expr_list_.empty(); }
    void clear();
    typedef std::list<processed_expression> expression_list;
    expression_list::const_iterator begin() const;
    expression_list::const_iterator end() const;
    string_info &get_string_info();
private:
    expression_list expr_list_;
    face_manager<freetype_engine> & font_manager_;
    double scale_factor_;
    string_info info_;
};


namespace formating {
class node;
typedef boost::shared_ptr<node> node_ptr;
class node
{
public:
    virtual ~node() {}
    virtual void to_xml(boost::property_tree::ptree &xml) const;
    static node_ptr from_xml(boost::property_tree::ptree const& xml);
    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const = 0;
    virtual void add_expressions(std::set<expression_ptr> &expressions) const;
};

class list_node: public node {
public:
    list_node() : node(), children_() {}
    virtual void to_xml(boost::property_tree::ptree &xml) const;
    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const;
    virtual void add_expressions(std::set<expression_ptr> &expressions) const;

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
    void to_xml(boost::property_tree::ptree &xml) const;
    static node_ptr from_xml(boost::property_tree::ptree const& xml);
    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const;
    virtual void add_expressions(std::set<expression_ptr> &expressions) const;

    void set_text(expression_ptr text);
    expression_ptr get_text() const;
private:
    expression_ptr text_;
};

class format_node: public node {
public:
    format_node();
    void to_xml(boost::property_tree::ptree &xml) const;
    static node_ptr from_xml(boost::property_tree::ptree const& xml);
    virtual void apply(char_properties const& p, Feature const& feature, processed_text &output) const;

    void set_child(node_ptr child);
    node_ptr get_child() const;

    void set_face_name(boost::optional<std::string> face_name);
    void set_text_size(boost::optional<unsigned> text_size);
    void set_character_spacing(boost::optional<unsigned> character_spacing);
    void set_line_spacing(boost::optional<unsigned> line_spacing);
    void set_text_opacity(boost::optional<double> opacity);
    void set_wrap_before(boost::optional<bool> wrap_before);
    void set_wrap_char(boost::optional<unsigned> wrap_char);
    void set_text_transform(boost::optional<text_transform_e> text_trans);
    void set_fill(boost::optional<color> fill);
    void set_halo_fill(boost::optional<color> halo_fill);
    void set_halo_radius(boost::optional<double> radius);
private:
    boost::optional<std::string> face_name_;
    boost::optional<unsigned> text_size_;
    boost::optional<unsigned> character_spacing_;
    boost::optional<unsigned> line_spacing_;
    boost::optional<double> text_opacity_;
    boost::optional<bool> wrap_before_;
    boost::optional<unsigned> wrap_char_;
    boost::optional<text_transform_e> text_transform_;
    boost::optional<color> fill_;
    boost::optional<color> halo_fill_;
    boost::optional<double> halo_radius_;
    node_ptr child_;
};

} //namespace formating

/** Stores formating information and uses this to produce formated text for a given feature. */
class text_processor
{
public:
    text_processor();
    /** Construct object from XML. */
    void from_xml(boost::property_tree::ptree const& pt, std::map<std::string,font_set> const &fontsets);
    /** Write object to XML ptree. */
    void to_xml(boost::property_tree::ptree &node,  bool explicit_defaults, text_processor const& dfl) const;
    
    /** Takes a feature and produces formated text as output. 
     * The output object has to be created by the caller and passed in for thread safety.
     */
    void process(processed_text &output, Feature const& feature) const;
    /** Automatically create processing instructions for a single expression. */
    void set_old_style_expression(expression_ptr expr);
    /** Sets new format tree. */
    void set_format_tree(formating::node_ptr tree);
    /** Get format tree. */
    formating::node_ptr get_format_tree() const;
    /** Get a list of all expressions used in any placement. This function is used to collect attributes. */
    std::set<expression_ptr> get_all_expressions() const;
    /** Default values for char_properties. */
    char_properties defaults;
private:
    formating::node_ptr tree_;
};

} /* namespace mapnik*/

#endif
