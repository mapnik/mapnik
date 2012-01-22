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
    void set_values_from_xml(boost::property_tree::ptree const &sym, std::map<std::string,font_set> const & fontsets);
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

class abstract_token;
class processed_text;

class processed_expression
{
public:
    processed_expression(char_properties const& properties, UnicodeString const& text) :
        p(properties), str(text) {}
    char_properties p;
    UnicodeString str;
private:
    friend class processed_text;
};

class processed_text
{
public:
    processed_text(face_manager<freetype_engine> & font_manager, double scale_factor);
    void push_back(processed_expression const& exp);
    unsigned size() const { return expr_list_.size(); }
    unsigned empty() const { return expr_list_.empty(); }
    void clear();
    typedef std::list<processed_expression> expression_list;
    expression_list::const_iterator begin();
    expression_list::const_iterator end();
    string_info &get_string_info();
private:
    expression_list expr_list_;
    face_manager<freetype_engine> & font_manager_;
    double scale_factor_;
    string_info info_;
};


class text_processor
{
public:
    text_processor();
    void from_xml(boost::property_tree::ptree const& pt, std::map<std::string,font_set> const &fontsets);
    void to_xml(boost::property_tree::ptree &node,  bool explicit_defaults, text_processor const& dfl) const;
    void process(processed_text &output, Feature const& feature);
    void set_old_style_expression(expression_ptr expr);
    void push_back(abstract_token *token);
    std::set<expression_ptr> get_all_expressions() const;
    char_properties defaults;
protected:
    void from_xml_recursive(boost::property_tree::ptree const& pt, std::map<std::string,font_set> const &fontsets);
private:
    std::list<abstract_token *> list_;
    bool clear_on_write; //Clear list once
};

} /* namespace */

#endif
