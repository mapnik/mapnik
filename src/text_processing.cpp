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
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/expression_string.hpp>

#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>

#include <stack>

namespace mapnik {
using boost::property_tree::ptree;
using boost::optional;

class abstract_token
{
public:
    virtual ~abstract_token() {}
    virtual ptree *to_xml(ptree *node) = 0;
};

class abstract_formating_token : public abstract_token
{
public:
    virtual void apply(char_properties &p, Feature const& feature) = 0;
};

class abstract_text_token : public abstract_token
{
public:
    virtual UnicodeString to_string(Feature const& feature) = 0;
};

class end_format_token : public abstract_token
{
public:
    end_format_token() {}
    ptree *to_xml(ptree *node);
};

class expression_token: public abstract_text_token
{
public:
    expression_token(expression_ptr text);
    UnicodeString to_string(Feature const& feature);
    ptree *to_xml(ptree *node);
    void set_expression(expression_ptr text);
    expression_ptr get_expression();
private:
    expression_ptr text_;
};

class fixed_formating_token : public abstract_formating_token
{
public:
    fixed_formating_token();
    virtual void apply(char_properties &p, Feature const& feature);
    ptree* to_xml(ptree *node);
    void from_xml(ptree const& node);
    void set_face_name(optional<std::string> face_name);
    void set_text_size(optional<unsigned> text_size);
    void set_character_spacing(optional<unsigned> character_spacing);
    void set_line_spacing(optional<unsigned> line_spacing);
    void set_text_opacity(optional<double> opacity);
    void set_wrap_before(optional<boolean> wrap_before);
    void set_wrap_char(optional<unsigned> wrap_char);
    void set_text_transform(optional<text_transform_e> text_trans);
    void set_fill(optional<color> fill);
    void set_halo_fill(optional<color> halo_fill);
    void set_halo_radius(optional<double> radius);
private:
    boost::optional<std::string> face_name_;
//    font_set fontset;
    boost::optional<unsigned> text_size_;
    boost::optional<unsigned> character_spacing_;
    boost::optional<unsigned> line_spacing_;
    boost::optional<double> text_opacity_;
    boost::optional<boolean> wrap_before_;
    boost::optional<unsigned> wrap_char_;
    boost::optional<text_transform_e> text_transform_;
    boost::optional<color> fill_;
    boost::optional<color> halo_fill_;
    boost::optional<double> halo_radius_;
};

/************************************************************/

expression_token::expression_token(expression_ptr text):
    text_(text)
{
}

void expression_token::set_expression(expression_ptr text)
{
    text_ = text;
}

expression_ptr expression_token::get_expression()
{
    return text_;
}

UnicodeString expression_token::to_string(const Feature &feature)
{
    value_type result = boost::apply_visitor(evaluate<Feature,value_type>(feature), *text_);
    return result.to_unicode();
}

ptree *expression_token::to_xml(ptree *node)
{
    ptree &new_node = node->push_back(ptree::value_type(
                             "<xmltext>", ptree()))->second;
    new_node.put_value(to_expression_string(*text_));
    return &new_node;
}

/************************************************************/

fixed_formating_token::fixed_formating_token():
    fill_()
{
}

void fixed_formating_token::apply(char_properties &p, const Feature &feature)
{
    if (face_name_) p.face_name = *face_name_;
    if (text_size_) p.text_size = *text_size_;
    if (character_spacing_) p.character_spacing = *character_spacing_;
    if (line_spacing_) p.line_spacing = *line_spacing_;
    if (text_opacity_) p.text_opacity = *text_opacity_;
    if (wrap_before_) p.wrap_before = *wrap_before_;
    if (wrap_char_) p.wrap_char = *wrap_char_;
    if (text_transform_) p.text_transform = *text_transform_;
    if (fill_) p.fill = *fill_;
    if (halo_fill_) p.halo_fill = *halo_fill_;
    if (halo_radius_) p.halo_radius = *halo_radius_;
}

ptree *fixed_formating_token::to_xml(ptree *node)
{

    ptree &new_node = node->push_back(ptree::value_type("Format", ptree()))->second;
    if (face_name_) set_attr(new_node, "face-name", face_name_);
    if (text_size_) set_attr(new_node, "size", text_size_);
    if (character_spacing_) set_attr(new_node, "character-spacing", character_spacing_);
    if (line_spacing_) set_attr(new_node, "line-spacing", line_spacing_);
    if (text_opacity_) set_attr(new_node, "opacity", text_opacity_);
    if (wrap_before_) set_attr(new_node, "wrap-before", wrap_before_);
    if (wrap_char_) set_attr(new_node, "wrap-character", wrap_char_);
    if (text_transform_) set_attr(new_node, "text-transform", text_transform_);
    if (fill_) set_attr(new_node, "fill", fill_);
    if (halo_fill_) set_attr(new_node, "halo-fill", halo_fill_);
    if (halo_radius_) set_attr(new_node, "halo-radius", halo_radius_);
    return &new_node;
}

void fixed_formating_token::from_xml(ptree const& node)
{
    set_face_name(get_opt_attr<std::string>(node, "face-name"));
    /*TODO: Fontset is problematic. We don't have the fontsets pointer here... */
    set_text_size(get_opt_attr<unsigned>(node, "size"));
    set_character_spacing(get_opt_attr<unsigned>(node, "character-spacing"));
    set_line_spacing(get_opt_attr<unsigned>(node, "line-spacing"));
    set_text_opacity(get_opt_attr<double>(node, "opactity"));
    set_wrap_before(get_opt_attr<boolean>(node, "wrap-before"));
    set_wrap_char(get_opt_attr<unsigned>(node, "wrap-character"));
    set_text_transform(get_opt_attr<text_transform_e>(node, "text-transform"));
    set_fill(get_opt_attr<color>(node, "fill"));
    set_halo_fill(get_opt_attr<color>(node, "halo-fill"));
    set_halo_radius(get_opt_attr<double>(node, "halo-radius"));
}

void fixed_formating_token::set_face_name(optional<std::string> face_name)
{
    face_name_ = face_name;
}

void fixed_formating_token::set_text_size(optional<unsigned> text_size)
{
    text_size_ = text_size;
}

void fixed_formating_token::set_character_spacing(optional<unsigned> character_spacing)
{
    character_spacing_ = character_spacing;
}

void fixed_formating_token::set_line_spacing(optional<unsigned> line_spacing)
{
    line_spacing_ = line_spacing;
}

void fixed_formating_token::set_text_opacity(optional<double> text_opacity)
{
    text_opacity_ = text_opacity;
}

void fixed_formating_token::set_wrap_before(optional<boolean> wrap_before)
{
    wrap_before_ = wrap_before;
}

void fixed_formating_token::set_wrap_char(optional<unsigned> wrap_char)
{
    wrap_char_ = wrap_char;
}

void fixed_formating_token::set_text_transform(optional<text_transform_e> text_transform)
{
    text_transform_ = text_transform;
}

void fixed_formating_token::set_fill(optional<color> c)
{
    fill_ = c;
}

void fixed_formating_token::set_halo_fill(optional<color> c)
{
    halo_fill_ = c;
}

void fixed_formating_token::set_halo_radius(optional<double> radius)
{
    halo_radius_ = radius;
}

/************************************************************/

ptree *end_format_token::to_xml(ptree *node)
{
    return 0;
}

/************************************************************/

text_processor::text_processor():
    list_(), clear_on_write(false)
{
}

void text_processor::push_back(abstract_token *token)
{
    if (clear_on_write) list_.clear();
    clear_on_write = false;
    list_.push_back(token);
}

void text_processor::from_xml(const boost::property_tree::ptree &pt, std::map<std::string,font_set> const &fontsets)
{
    clear_on_write = true;
    defaults.set_values_from_xml(pt, fontsets);
    from_xml_recursive(pt, fontsets);
}

void text_processor::from_xml_recursive(const boost::property_tree::ptree &pt, std::map<std::string,font_set> const &fontsets)
{
    ptree::const_iterator itr = pt.begin();
    ptree::const_iterator end = pt.end();
    for (; itr != end; ++itr) {
        if (itr->first == "<xmltext>") {
            std::string data = itr->second.data();
            boost::trim(data);
            if (data.empty()) continue;
            expression_token *token = new expression_token(parse_expression(data, "utf8"));
            push_back(token);
        } else if (itr->first == "Format") {
            fixed_formating_token *token = new fixed_formating_token();
            token->from_xml(itr->second);
            push_back(token);
            from_xml_recursive(itr->second, fontsets); /* Parse children, making a list out of a tree. */
            push_back(new end_format_token());
        } else if (itr->first != "<xmlcomment>" && itr->first != "<xmlattr>" && itr->first != "Placement") {
            std::cerr << "Unknown item" << itr->first;
        }
    }
}

void text_processor::to_xml(boost::property_tree::ptree &node, bool explicit_defaults, text_processor const& dfl) const
{
    defaults.to_xml(node, explicit_defaults, dfl.defaults);
    std::list<abstract_token *>::const_iterator itr = list_.begin();
    std::list<abstract_token *>::const_iterator end = list_.end();
    std::stack<ptree *> nodes;
    ptree *current_node = &node;
    for (; itr != end; ++itr) {
        abstract_token *token = *itr;
        ptree *new_node = token->to_xml(current_node);
        if (dynamic_cast<abstract_formating_token *>(token)) {
            nodes.push(current_node);
            current_node = new_node;
        } else if (dynamic_cast<end_format_token *>(token)) {
            current_node = nodes.top();
            nodes.pop();
        }
    }
}

void text_processor::process(processed_text &output, Feature const& feature)
{
    std::list<abstract_token *>::const_iterator itr = list_.begin();
    std::list<abstract_token *>::const_iterator end = list_.end();
    std::stack<char_properties> formats;
    formats.push(defaults);

    for (; itr != end; ++itr) {
        abstract_text_token *text = dynamic_cast<abstract_text_token *>(*itr);
        abstract_formating_token *format = dynamic_cast<abstract_formating_token *>(*itr);;
        end_format_token *end = dynamic_cast<end_format_token *>(*itr);;
        if (text) {
            UnicodeString text_str = text->to_string(feature);
            char_properties const& p = formats.top();
            /* TODO: Make a class out of text_transform which does the work! */
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
                output.push_back(processed_expression(p, text_str));
            } else {
#ifdef MAPNIK_DEBUG
                std::cerr << "Warning: Empty expression.\n";
#endif
            }
        } else if (format) {
            char_properties next_properties = formats.top();
            format->apply(next_properties, feature);
            formats.push(next_properties);
        } else if (end) {
            /* Always keep at least the defaults_ on stack. */
            if (formats.size() > 1) {
                formats.pop();
            } else {
                std::cerr << "Warning: Internal mapnik error. More elements popped than pushed in text_processor::process()\n";
                output.clear();
                return;
            }
        }
    }
    if (formats.size() != 1) {
        std::cerr << "Warning: Internal mapnik error. Less elements popped than pushed in text_processor::process()\n";
    }
}

std::set<expression_ptr> text_processor::get_all_expressions() const
{
    std::set<expression_ptr> result;
    std::list<abstract_token *>::const_iterator itr = list_.begin();
    std::list<abstract_token *>::const_iterator end = list_.end();
    for (; itr != end; ++itr) {
        expression_token *text = dynamic_cast<expression_token *>(*itr);
        if (text) result.insert(text->get_expression());
    }
    return result;
}

void text_processor::set_old_style_expression(expression_ptr expr)
{
    list_.push_back(new expression_token(expr));
}

/************************************************************/

void processed_text::push_back(processed_expression const& exp)
{
    expr_list_.push_back(exp);
}

processed_text::expression_list::const_iterator processed_text::begin()
{
    return expr_list_.begin();
}

processed_text::expression_list::const_iterator processed_text::end()
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
    //info_.clear(); TODO: if this function is called twice invalid results are returned, so clear string_info first
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
        faces->set_pixel_sizes(p.text_size * scale_factor_);
        faces->get_string_info(info_, itr->str, &(itr->p));
        info_.add_text(itr->str);
    }
    return info_;
}



} /* namespace */
