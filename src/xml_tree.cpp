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

//mapnik
#include <mapnik/xml_tree.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/enumeration.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/gamma_method.hpp>
#include <mapnik/line_symbolizer.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/text_properties.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/raster_colorizer.hpp>

//boost
#include <boost/lexical_cast.hpp>

namespace mapnik
{

template <typename T>
inline boost::optional<T> fast_cast(xml_tree const& tree, std::string const& value)
{
    try
    {
        return boost::lexical_cast<T>(value);
    }
    catch (boost::bad_lexical_cast const& ex)
    {
        return boost::optional<T>();
    }
}

template <>
inline boost::optional<int> fast_cast(xml_tree const& tree, std::string const& value)
{
    int result;
    if (mapnik::util::string2int(value, result))
        return boost::optional<int>(result);
    return boost::optional<int>();
}

template <>
inline boost::optional<double> fast_cast(xml_tree const& tree, std::string const& value)
{
    double result;
    if (mapnik::util::string2double(value, result))
        return boost::optional<double>(result);
    return boost::optional<double>();
}

template <>
inline boost::optional<float> fast_cast(xml_tree const& tree, std::string const& value)
{
    float result;
    if (mapnik::util::string2float(value, result))
        return boost::optional<float>(result);
    return boost::optional<float>();
}

template <>
inline boost::optional<std::string> fast_cast(xml_tree const& tree, std::string const& value)
{
    return value;
}

template <>
inline boost::optional<color> fast_cast(xml_tree const& tree, std::string const& value)
{
    return parse_color(value, tree.color_grammar);
}

template <>
inline boost::optional<expression_ptr> fast_cast(xml_tree const& tree, std::string const& value)
{
    return parse_expression(value, tree.expr_grammar);
}

/****************************************************************************/

class boolean;
template <typename T>
struct name_trait
{
    static std::string name()
    {
        return "<unknown>";
    }
    // missing name_trait for type ...
    // if you get here you are probably using a new type
    // in the XML file. Just add a name trait for the new
    // type below.
    BOOST_STATIC_ASSERT( sizeof(T) == 0 );
};

#define DEFINE_NAME_TRAIT( type, type_name )                            \
    template <>                                                         \
    struct name_trait<type>                                             \
    {                                                                   \
        static std::string name() { return std::string("type ") + type_name; } \
    };


DEFINE_NAME_TRAIT( double, "double")
DEFINE_NAME_TRAIT( float, "float")
DEFINE_NAME_TRAIT( unsigned, "unsigned")
DEFINE_NAME_TRAIT( boolean, "boolean")
DEFINE_NAME_TRAIT( int, "integer" )
DEFINE_NAME_TRAIT( std::string, "string" )
DEFINE_NAME_TRAIT( color, "color" )
DEFINE_NAME_TRAIT(expression_ptr, "expression_ptr" )

template <typename ENUM, int MAX>
struct name_trait< mapnik::enumeration<ENUM, MAX> >
{
    typedef enumeration<ENUM, MAX> Enum;

    static std::string name()
    {
        std::string value_list("one of [");
        for (unsigned i = 0; i < Enum::MAX; ++i)
        {
            value_list += Enum::get_string( i );
            if ( i + 1 < Enum::MAX ) value_list += ", ";
        }
        value_list += "]";

        return value_list;
    }
};

/****************************************************************************/

xml_tree::xml_tree(std::string const& encoding)
    : node_(*this, "<root>"),
      file_(),
      tr_(encoding),
      color_grammar(),
      expr_grammar(tr_),
      path_expr_grammar(),
      transform_expr_grammar(expr_grammar),
      image_filters_grammar()
{
    node_.set_processed(true); //root node is always processed
}

void xml_tree::set_filename(std::string fn)
{
    file_ = fn;
}

std::string const& xml_tree::filename() const
{
    return file_;
}

xml_node &xml_tree::root()
{
    return node_;
}

const xml_node &xml_tree::root() const
{
    return node_;
}

/****************************************************************************/
xml_attribute::xml_attribute(std::string const& value_)
    : value(value_), processed(false)
{

}

/****************************************************************************/

node_not_found::node_not_found(std::string const& node_name)
    : node_name_(node_name)
{

}

const char* node_not_found::what() const throw()
{
    return ("Node "+node_name_+ "not found").c_str();
}

node_not_found::~node_not_found() throw()
{

}


attribute_not_found::attribute_not_found(
    std::string const& node_name,
    std::string const& attribute_name)
    :
    node_name_(node_name),
    attribute_name_(attribute_name)
{

}

const char* attribute_not_found::what() const throw()
{
    return ("Attribute '" + attribute_name_ +"' not found in node '"+node_name_+ "'").c_str();
}

attribute_not_found::~attribute_not_found() throw()
{

}

more_than_one_child::more_than_one_child(std::string const& node_name)
    : node_name_(node_name)
{

}

const char* more_than_one_child::what() const throw()
{
    return ("More than one child node in node '" + node_name_ +"'").c_str();
}

more_than_one_child::~more_than_one_child() throw()
{

}

/****************************************************************************/

xml_node::xml_node(xml_tree &tree, std::string const& name, unsigned line, bool is_text)
    : tree_(tree),
      name_(name),
      is_text_(is_text),
      line_(line),
      processed_(false)
{

}

std::string xml_node::xml_text = "<xmltext>";

std::string const& xml_node::name() const
{
    if (!is_text_)
        return name_;
    else
        return xml_text;
}

std::string const& xml_node::text() const
{
    if (is_text_)
    {
        processed_ = true;
        return name_;
    } else
    {
        throw config_error("text() called on non-text node", *this);
    }
}

std::string const& xml_node::filename() const
{
    return tree_.filename();
}

bool xml_node::is_text() const
{
    return is_text_;
}

bool xml_node::is(std::string const& name) const
{
    if (name_ == name)
    {
        processed_ = true;
        return true;
    }
    return false;
}

xml_node &xml_node::add_child(std::string const& name, unsigned line, bool is_text)
{
    children_.push_back(xml_node(tree_, name, line, is_text));
    return children_.back();
}

void xml_node::add_attribute(std::string const& name, std::string const& value)
{
    attributes_.insert(std::make_pair(name,xml_attribute(value)));
}

xml_node::attribute_map const& xml_node::get_attributes() const
{
    return attributes_;
}

void xml_node::set_processed(bool processed) const
{
    processed_ = processed;
}

bool xml_node::processed() const
{
    return processed_;
}

xml_node::const_iterator xml_node::begin() const
{
    return children_.begin();
}

xml_node::const_iterator xml_node::end() const
{
    return children_.end();
}

xml_node & xml_node::get_child(std::string const& name)
{
    std::list<xml_node>::iterator itr = children_.begin();
    std::list<xml_node>::iterator end = children_.end();
    for (; itr != end; itr++)
    {
        if (!(itr->is_text_) && itr->name_ == name)
        {
            itr->set_processed(true);
            return *itr;
        }
    }
    throw node_not_found(name);
}

xml_node const& xml_node::get_child(std::string const& name) const
{
    xml_node const* node = get_opt_child(name);
    if (!node) throw node_not_found(name);
    return *node;
}

xml_node const* xml_node::get_opt_child(std::string const& name) const
{
    const_iterator itr = children_.begin();
    const_iterator end = children_.end();
    for (; itr != end; itr++)
    {
        if (!(itr->is_text_) && itr->name_ == name)
        {
            itr->set_processed(true);
            return &(*itr);
        }
    }
    return 0;
}

bool xml_node::has_child(std::string const& name) const
{
    return get_opt_child(name) != 0;
}

template <typename T>
boost::optional<T> xml_node::get_opt_attr(std::string const& name) const
{
    std::map<std::string, xml_attribute>::const_iterator itr = attributes_.find(name);
    if (itr ==  attributes_.end()) return boost::optional<T>();
    itr->second.processed = true;
    boost::optional<T> result = fast_cast<T>(tree_, itr->second.value);
    if (!result)
    {
        throw config_error(std::string("Failed to parse attribute '") +
                           name + "'. Expected " + name_trait<T>::name() +
                           " but got '" + itr->second.value + "'", *this);
    }
    return result;
}

template <typename T>
T xml_node::get_attr(std::string const& name, T const& default_value) const
{
    boost::optional<T> value = get_opt_attr<T>(name);
    if (value) return *value;
    return default_value;
}

template <typename T>
T xml_node::get_attr(std::string const& name) const
{
    boost::optional<T> value = get_opt_attr<T>(name);
    if (value) return *value;
    throw attribute_not_found(name_, name);
}

std::string xml_node::get_text() const
{
    if (children_.size() == 0)
    {
        if (is_text_)
        {
            return name_;
        } else
        {
            return "";
        }
    }
    if (children_.size() == 1)
    {
        return children_.front().text();
    }
    throw more_than_one_child(name_);
}


template <typename T>
T xml_node::get_value() const
{
    boost::optional<T> result = fast_cast<T>(tree_, get_text());
    if (!result)
    {
        throw config_error(std::string("Failed to parse value. Expected ")
                           + name_trait<T>::name() +
                           " but got '" + get_text() + "'", *this);
    }
    return *result;
}

unsigned xml_node::line() const
{
    return line_;
}

#define compile_get_opt_attr(T) template boost::optional<T> xml_node::get_opt_attr<T>(std::string const&) const
#define compile_get_attr(T) template T xml_node::get_attr<T>(std::string const&) const; template T xml_node::get_attr<T>(std::string const&, T const&) const
#define compile_get_value(T) template T xml_node::get_value<T>() const

compile_get_opt_attr(boolean);
compile_get_opt_attr(std::string);
compile_get_opt_attr(unsigned);
compile_get_opt_attr(float);
compile_get_opt_attr(double);
compile_get_opt_attr(color);
compile_get_opt_attr(gamma_method_e);
compile_get_opt_attr(line_join_e);
compile_get_opt_attr(line_cap_e);
compile_get_opt_attr(text_transform_e);
compile_get_opt_attr(label_placement_e);
compile_get_opt_attr(vertical_alignment_e);
compile_get_opt_attr(horizontal_alignment_e);
compile_get_opt_attr(justify_alignment_e);
compile_get_opt_attr(expression_ptr);
compile_get_attr(std::string);
compile_get_attr(filter_mode_e);
compile_get_attr(point_placement_e);
compile_get_attr(marker_placement_e);
compile_get_attr(pattern_alignment_e);
compile_get_attr(line_rasterizer_e);
compile_get_attr(colorizer_mode);
compile_get_attr(double);
compile_get_value(int);
compile_get_value(double);
compile_get_value(expression_ptr);
} //ns mapnik
