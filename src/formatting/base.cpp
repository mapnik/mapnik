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
#include <mapnik/formatting/base.hpp>
#include <mapnik/formatting/list.hpp>
#include <mapnik/formatting/text.hpp>
#include <mapnik/formatting/expression.hpp>
#include <mapnik/formatting/format.hpp>

// boost
#include <boost/property_tree/ptree.hpp>

namespace mapnik {
namespace formatting {

void node::to_xml(boost::property_tree::ptree &xml) const
{
    //TODO: Should this throw a config_error?
#ifdef MAPNIK_DEBUG
    std::cerr << "Error: Trying to write unsupported node type to XML.\n";
#endif
}

node_ptr node::from_xml(boost::property_tree::ptree const& xml)
{
    list_node *list = new list_node();
    node_ptr list_ptr(list);
    boost::property_tree::ptree::const_iterator itr = xml.begin();
    boost::property_tree::ptree::const_iterator end = xml.end();
    for (; itr != end; ++itr) {
        node_ptr n;
        if (itr->first == "<xmltext>") {
            n = text_node::from_xml(itr->second);
        } else if (itr->first == "Format") {
            n = format_node::from_xml(itr->second);
        } else if (itr->first == "ExpressionFormat") {
            n = expression_format::from_xml(itr->second);
        } else if (itr->first != "<xmlcomment>" && itr->first != "<xmlattr>" && itr->first != "Placement") {
            throw config_error("Unknown item " + itr->first);
        }
        if (n) list->push_back(n);
    }
    if (list->get_children().size() == 1) {
        return list->get_children()[0];
    } else if (list->get_children().size() > 1) {
        return list_ptr;
    } else {
        return node_ptr();
    }
}

void node::add_expressions(expression_set &output) const
{
    //Do nothing by default
}

} //ns formatting
} //ns mapnik
