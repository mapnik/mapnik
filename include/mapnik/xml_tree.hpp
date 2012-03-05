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

#include <list>
#include <string>
#include <map>

namespace mapnik
{
class xml_tree;

class xml_attribute
{
public:
    std::string value;
    bool processed;
};

class xml_node
{
public:
    xml_node(xml_tree &tree, std::string name, unsigned line=0, bool text_node = false);

    std::string name() const;
    std::string text() const;

    xml_node &add_child(std::string name, unsigned line=0, bool text_node = false);
    void add_attribute(std::string name, std::string value);
    void set_processed(bool processed);
private:
    xml_tree &tree_;
    std::string name_;
    std::list<xml_node> children_;
    std::map<std::string, xml_attribute> attributes_;
    bool text_node_;
    unsigned line_;
    bool processed_;

};

class xml_tree
{
public:
    xml_tree();
    void set_filename(std::string fn);
    std::string filename() const;
    xml_node &node();
private:
    xml_node node_;
    std::string file_;
    //TODO: Grammars
};

} //ns mapnik

#endif // MAPNIK_XML_TREE_H
