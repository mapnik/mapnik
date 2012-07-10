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
#include <mapnik/debug.hpp>
#include <mapnik/formatting/offset.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/processed_text.hpp>
#include <mapnik/xml_node.hpp>

namespace mapnik {
namespace formatting {

using boost::property_tree::ptree;

void offset_node::to_xml(ptree &xml) const
{
        ptree &new_node = xml.push_back(ptree::value_type("Offset", ptree()))->second;
        set_attr(new_node, "dx", displacement.first);
        set_attr(new_node, "dy", displacement.second);
        if (child_) child_->to_xml(new_node);
}

node_ptr offset_node::from_xml(xml_node const& xml)
{
    offset_node *n = new offset_node();
    node_ptr np(n);

    node_ptr child = node::from_xml(xml);
    n->set_child(child);
    
    boost::optional<double> dx = xml.get_opt_attr<double>("dx");
    if (dx) n->displacement.first = *dx;
    boost::optional<double> dy = xml.get_opt_attr<double>("dy");
    if (dy) n->displacement.second = *dy;

    return np;
}

void offset_node::apply(char_properties const& p, const Feature &feature, processed_text &output) const
{
    // add displacement x/y of the child to displacement x/y of the parent
    position child_displacement = output.get_displacement();
    child_displacement.first += displacement.first;
    child_displacement.second += displacement.second;
    
    // starting a new offset child with the new displacement value
    processed_text_ptr offset_child(new processed_text(output.get_font_manager(), output.get_scale_factor(), child_displacement));

    // process contained format tree into the child node
    if (child_) {
        child_->apply(p, feature, *offset_child);
    } else {
        MAPNIK_LOG_WARN(format) << "Useless offset node: Contains no text";
    }
    
    // add child to parent
    output.add_child(offset_child);
}

void offset_node::set_child(node_ptr child)
{
    child_ = child;
}


node_ptr offset_node::get_child() const
{
    return child_;
}

void offset_node::add_expressions(expression_set &output) const
{
    if (child_) child_->add_expressions(output);
}

} //ns formatting
} //ns mapnik
