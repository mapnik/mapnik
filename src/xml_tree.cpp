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

//boost
#include <boost/lexical_cast.hpp>

namespace mapnik
{

template <typename T>
inline boost::optional<T> fast_cast(xml_tree const& tree, std::string const& value)
{
    return boost::lexical_cast<T>(value);
}

template <>
inline boost::optional<int> fast_cast(xml_tree const& tree, std::string const& value)
{
    int result;
    if (mapnik::conversions::string2int(value, result))
        return boost::optional<int>(result);
    return boost::optional<int>();
}

template <>
inline boost::optional<double> fast_cast(xml_tree const& tree, std::string const& value)
{
    double result;
    if (mapnik::conversions::string2double(value, result))
        return boost::optional<double>(result);
    return boost::optional<double>();
}

template <>
inline boost::optional<float> fast_cast(xml_tree const& tree, std::string const& value)
{
    float result;
    if (mapnik::conversions::string2float(value, result))
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
    return mapnik::color_factory::from_string(value);
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

#define DEFINE_NAME_TRAIT( type, type_name )                  \
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

xml_tree::xml_tree()
    : node_(*this, "<root>")
{
}

void xml_tree::set_filename(std::string fn)
{
    file_ = fn;
}

std::string xml_tree::filename() const
{
    return file_;
}

xml_node &xml_tree::root()
{
    return node_;
}

/****************************************************************************/

xml_node::xml_node(xml_tree &tree, std::string name, unsigned line, bool text_node)
    : tree_(tree),
      name_(name),
      text_node_(text_node),
      line_(line),
      processed_(false)
{

}

std::string xml_node::name() const
{
    if (!text_node_)
        return name_;
    else
        return "<xmltext>"; //TODO: throw
}

std::string xml_node::text() const
{
    if (text_node_)
        return name_;
    else
        return "NOT A TEXT NODE"; //TODO: throw
}

void xml_node::set_processed(bool processed)
{
    processed_ = processed;
}

xml_node &xml_node::add_child(std::string name, unsigned line, bool text_node)
{
    children_.push_back(xml_node(tree_, name, line, text_node));
    return children_.back();
}

xml_node & xml_node::get_child(std::string name)
{
    std::list<xml_node>::iterator itr = children_.begin();
    std::list<xml_node>::iterator end = children_.end();
    for (; itr != end; itr++)
    {
        if (!(itr->text_node_) && itr->name_ == name)
        {
            itr->set_processed(true);
            return *itr;
        }
    }
    throw node_not_found(name);
}

template <typename T>
boost::optional<T> xml_node::get_opt_attr(std::string const& name) const
{
    std::map<std::string, xml_attribute>::const_iterator itr = attributes_.find(name);
    if (itr ==  attributes_.end()) return boost::optional<T>();
    boost::optional<T> result = fast_cast<T>(itr->second);
    if (!result)
    {
        throw config_error(std::string("Failed to parse attribute '") +
                           name + "'. Expected " + name_trait<T>::name() +
                           " but got '" + itr->second + "'");
    }
    return result;
}


} //ns mapnik
