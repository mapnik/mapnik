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

// mapnik
#include <mapnik/formatting/list.hpp>
// boost
#include <boost/foreach.hpp>

namespace mapnik {
using boost::property_tree::ptree;

namespace formatting {
/************************************************************/

void list_node::to_xml(boost::property_tree::ptree & xml) const
{
    BOOST_FOREACH(node_ptr const& node, children_)
    {
        node->to_xml(xml);
    }
}


void list_node::apply(char_properties const& p, Feature const& feature, processed_text &output) const
{    
    BOOST_FOREACH(node_ptr const& node, children_)
    {
        node->apply(p, feature, output);
    }   
}


void list_node::add_expressions(expression_set &output) const
{
    BOOST_FOREACH(node_ptr const& node, children_)
    {
        node->add_expressions(output);
    }
}

void list_node::push_back(node_ptr n)
{
    children_.push_back(n);
}

void list_node::clear()
{
    children_.clear();
}

void list_node::set_children(std::vector<node_ptr> const& children)
{
    children_ = children;
}

std::vector<node_ptr> const& list_node::get_children() const
{
    return children_;
}
} // ns mapnik
} // ns formatting

