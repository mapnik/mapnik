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
#include <mapnik/xml_tree.hpp>

namespace mapnik
{

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

xml_node &xml_tree::node()
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
        return "<xmltext>";
}

std::string xml_node::text() const
{
    if (text_node_)
        return name_;
    else
        return "NOT A TEXT NODE";
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


} //ns mapnik
