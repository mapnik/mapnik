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

#ifndef MAPNIK_XML_NODE_H
#define MAPNIK_XML_NODE_H

//boost
#include <boost/optional.hpp>

//stl
#include <list>
#include <string>
#include <map>
#include <exception>

namespace mapnik
{
class xml_tree;

class xml_attribute
{
public:
    xml_attribute(const char * value_);
    std::string value;
    mutable bool processed;
};

class node_not_found: public std::exception
{
public:
    node_not_found(std::string const& node_name);
    virtual const char* what() const throw();
    ~node_not_found() throw ();
private:
    std::string node_name_;
protected:
    mutable std::string msg_;
};

class attribute_not_found: public std::exception
{
public:
    attribute_not_found(std::string const& node_name, std::string const& attribute_name);
    virtual const char* what() const throw();
    ~attribute_not_found() throw ();
private:
    std::string node_name_;
    std::string attribute_name_;
protected:
    mutable std::string msg_;
};

class more_than_one_child: public std::exception
{
public:
    more_than_one_child(std::string const& node_name);
    virtual const char* what() const throw();
    ~more_than_one_child() throw ();
private:
    std::string node_name_;
protected:
    mutable std::string msg_;
};

class xml_node
{
public:
    using const_iterator = std::list<xml_node>::const_iterator;
    using attribute_map = std::map<std::string, xml_attribute>;
    xml_node(xml_tree &tree, std::string && name, unsigned line=0, bool is_text = false);

    std::string const& name() const;
    std::string const& text() const;
    std::string const& filename() const;
    bool is_text() const;
    bool is(std::string const& name) const;

    xml_node & add_child(const char * name, unsigned line=0, bool is_text = false);
    void add_attribute(const char * name, const char * value);
    attribute_map const& get_attributes() const;

    bool ignore() const;
    void set_ignore(bool ignore) const;

    bool processed() const;
    void set_processed(bool processed) const;

    unsigned line() const;
    std::string line_to_string() const;

    std::size_t size() const;
    const_iterator begin() const;
    const_iterator end() const;

    xml_node & get_child(std::string const& name);
    xml_node const& get_child(std::string const& name) const;
    xml_node const* get_opt_child(std::string const& name) const;
    bool has_child(std::string const& name) const;
    bool has_attribute(std::string const& name) const;

    template <typename T>
    boost::optional<T> get_opt_attr(std::string const& name) const;

    template <typename T>
    T get_attr(std::string const& name, T const& default_opt_value) const;
    template <typename T>
    T get_attr(std::string const& name) const;

    std::string const& get_text() const;

    inline xml_tree const& get_tree() const { return tree_; }

    template <typename T>
    T get_value() const;
private:
    xml_tree & tree_;
    std::string name_;
    std::list<xml_node> children_;
    attribute_map attributes_;
    bool is_text_;
    unsigned line_;
    mutable bool processed_;
    mutable bool ignore_;
    static std::string xml_text;
};

} //ns mapnik

#endif // MAPNIK_XML_NODE_H
