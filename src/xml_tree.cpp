/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/debug.hpp>
#include <mapnik/make_unique.hpp>
#include <mapnik/xml_tree.hpp>
#include <mapnik/xml_attribute_cast.hpp>
#include <mapnik/util/conversions.hpp>
#include <mapnik/enumeration.hpp>
#include <mapnik/color_factory.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/text/text_properties.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/raster_colorizer.hpp>
#include <mapnik/expression.hpp>
#include <mapnik/text/font_feature_settings.hpp>

// stl
#include <type_traits>

namespace mapnik
{

class boolean_type;
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
    static_assert( sizeof(T) == 0, "missing name_trait for the type");
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
DEFINE_NAME_TRAIT( int, "int")
DEFINE_NAME_TRAIT( bool, "bool")
DEFINE_NAME_TRAIT( boolean_type, "boolean_type")
#ifdef BIGINT
DEFINE_NAME_TRAIT( mapnik::value_integer, "long long" )
#endif
DEFINE_NAME_TRAIT( std::string, "string" )
DEFINE_NAME_TRAIT( color, "color" )
DEFINE_NAME_TRAIT( expression_ptr, "expression_ptr" )
DEFINE_NAME_TRAIT( font_feature_settings, "font-feature-settings" )

template <typename ENUM, int MAX>
struct name_trait< mapnik::enumeration<ENUM, MAX> >
{
    using Enum = enumeration<ENUM, MAX>;

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

xml_tree::xml_tree()
    : node_(*this, "<root>"),
      file_()
{
    node_.set_processed(true); //root node is always processed
}

void xml_tree::set_filename(std::string const& fn)
{
    file_ = fn;
}

std::string const& xml_tree::filename() const
{
    return file_;
}

xml_node & xml_tree::root()
{
    return node_;
}

const xml_node & xml_tree::root() const
{
    return node_;
}

xml_attribute::xml_attribute(const char * value_)
    : value(value_),
      processed(false) {}

node_not_found::node_not_found(std::string const& node_name)
    : node_name_(node_name),
      msg_() {}

const char* node_not_found::what() const throw()
{
    msg_ = std::string("Node "+node_name_+ "not found");
    return msg_.c_str();
}

node_not_found::~node_not_found() throw() {}


attribute_not_found::attribute_not_found(std::string const& node_name,
                                         std::string const& attribute_name)
    : node_name_(node_name),
      attribute_name_(attribute_name),
      msg_() {}

const char* attribute_not_found::what() const throw()
{
    msg_ = std::string("Attribute '" + attribute_name_ +"' not found in node '"+node_name_+ "'");
    return msg_.c_str();
}

attribute_not_found::~attribute_not_found() throw() {}

more_than_one_child::more_than_one_child(std::string const& node_name)
    : node_name_(node_name),
      msg_() {}

const char* more_than_one_child::what() const throw()
{
    msg_ = std::string("More than one child node in node '" + node_name_ +"'");
    return msg_.c_str();
}

more_than_one_child::~more_than_one_child() throw() {}

xml_node::xml_node(xml_tree &tree, std::string && name, unsigned line, bool is_text)
    : tree_(tree),
      name_(std::move(name)),
      is_text_(is_text),
      line_(line),
      processed_(false),
      ignore_(false) {}

std::string xml_node::xml_text = "<xmltext>";

std::string const& xml_node::name() const
{
    if (!is_text_)
    {
        return name_;
    }
    else
    {
        return xml_text;
    }
}

std::string const& xml_node::text() const
{
    if (is_text_)
    {
        processed_ = true;
        return name_;
    }
    else
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

xml_node & xml_node::add_child(const char * name, unsigned line, bool is_text)
{
    children_.emplace_back(tree_, name, line, is_text);
    return children_.back();
}

void xml_node::add_attribute(const char * name, const char * value)
{
    auto result = attributes_.emplace(name,xml_attribute(value));
    if (!result.second)
    {
        MAPNIK_LOG_ERROR(xml_tree) << "ignoring duplicate attribute '" << name << "'";
    }
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

void xml_node::set_ignore(bool ignore) const
{
    ignore_ = ignore;
}

bool xml_node::ignore() const
{
    return ignore_;
}

std::size_t xml_node::size() const
{
    return children_.size();
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
    for (; itr != end; ++itr)
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

bool xml_node::has_attribute(std::string const& name) const
{
    return attributes_.count(name) == 1 ? true : false;
}

template <typename T>
boost::optional<T> xml_node::get_opt_attr(std::string const& name) const
{
    if (attributes_.empty()) return boost::optional<T>();
    std::map<std::string, xml_attribute>::const_iterator itr = attributes_.find(name);
    if (itr ==  attributes_.end()) return boost::optional<T>();
    itr->second.processed = true;
    boost::optional<T> result = xml_attribute_cast<T>(tree_, std::string(itr->second.value));
    if (!result)
    {
        throw config_error(std::string("Failed to parse attribute '") +
                           name + "'. Expected " + name_trait<T>::name() +
                           " but got '" + itr->second.value + "'", *this);
    }
    return result;
}

template <typename T>
T xml_node::get_attr(std::string const& name, T const& default_opt_value) const
{
    boost::optional<T> value = get_opt_attr<T>(name);
    if (value) return *value;
    return default_opt_value;
}

template <typename T>
T xml_node::get_attr(std::string const& name) const
{
    boost::optional<T> value = get_opt_attr<T>(name);
    if (value) return *value;
    throw attribute_not_found(name_, name);
}

std::string const& xml_node::get_text() const
{
    // FIXME : return boost::optional<std::string const&>
    if (children_.empty())
    {
        if (is_text_)
        {
            return name_;
        }
        else
        {
            const static std::string empty;
            return empty;
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
    boost::optional<T> result = xml_attribute_cast<T>(tree_, get_text());
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

std::string xml_node::line_to_string() const
{
    std::string number;
    util::to_string(number,line_);
    return number;
}


#define compile_get_opt_attr(T) template boost::optional<T> xml_node::get_opt_attr<T>(std::string const&) const
#define compile_get_attr(T) template T xml_node::get_attr<T>(std::string const&) const; template T xml_node::get_attr<T>(std::string const&, T const&) const
#define compile_get_value(T) template T xml_node::get_value<T>() const

compile_get_opt_attr(boolean_type);
compile_get_opt_attr(mapnik::value_bool);
compile_get_opt_attr(std::string);
compile_get_opt_attr(int);
compile_get_opt_attr(unsigned);
#ifdef BIGINT
compile_get_opt_attr(mapnik::value_integer);
#endif
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
compile_get_opt_attr(text_upright_e);
compile_get_opt_attr(direction_e);
compile_get_opt_attr(halo_rasterizer_e);
compile_get_opt_attr(expression_ptr);
compile_get_opt_attr(font_feature_settings);
compile_get_attr(std::string);
compile_get_attr(filter_mode_e);
compile_get_attr(point_placement_e);
compile_get_attr(debug_symbolizer_mode_e);
compile_get_attr(marker_placement_e);
compile_get_attr(marker_multi_policy_e);
compile_get_attr(pattern_alignment_e);
compile_get_attr(line_rasterizer_e);
compile_get_attr(colorizer_mode);
compile_get_attr(double);
compile_get_value(value_integer);
compile_get_value(double);
compile_get_value(expression_ptr);
} //ns mapnik
