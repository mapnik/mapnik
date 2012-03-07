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

#ifndef MAPNIK_XML_TREE_H
#define MAPNIK_XML_TREE_H

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
class color;

class xml_attribute
{
public:
    std::string value;
    mutable bool processed;
};

class node_not_found: public std::exception
{
public:
    node_not_found(std::string node_name) : node_name_(node_name) {}
    virtual const char* what() const throw()
    {
        return ("Node "+node_name_+ "not found").c_str();
    }
    ~node_not_found() throw ();
private:
    std::string node_name_;
};

class xml_node
{
public:
    typedef std::list<xml_node>::const_iterator const_iterator;
    xml_node(xml_tree &tree, std::string name, unsigned line=0, bool text_node = false);

    std::string name() const;
    std::string text() const;
    bool is_text() const;
    bool is(std::string const& name) const;

    xml_node &add_child(std::string const& name, unsigned line=0, bool text_node = false);
    void add_attribute(std::string const& name, std::string const& value);
    void set_processed(bool processed);

    const_iterator begin() const;
    const_iterator end() const;

    xml_node & get_child(std::string const& name);
    xml_node const& get_child(std::string const& name) const;
    xml_node *get_opt_child(std::string const& name) const;
    bool has_child(std::string const& name) const;

    template <typename T>
    boost::optional<T> get_opt_attr(std::string const& name) const;

    template <typename T>
    T get_attr(std::string const& name, T const& default_value) const;
    template <typename T>
    T get_attr(std::string const& name) const;

    std::string get_text() const;

    template <typename T>
    T get_value() const;
private:
    xml_tree &tree_;
    std::string name_;
    std::list<xml_node> children_;
    std::map<std::string, xml_attribute> attributes_;
    bool text_node_;
    unsigned line_;
    mutable bool processed_;

};

class xml_tree
{
public:
    xml_tree();
    void set_filename(std::string fn);
    std::string filename() const;
    xml_node &root();
private:
    xml_node node_;
    std::string file_;
    //TODO: Grammars
};

} //ns mapnik

#endif // MAPNIK_XML_TREE_H
