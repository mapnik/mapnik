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
#include <mapnik/text/text_properties.hpp>
#include <mapnik/text/layout.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/expression_string.hpp>
#include <mapnik/text/formatting/layout.hpp>
#include <mapnik/xml_node.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/symbolizer.hpp>

// boost
#include <boost/property_tree/ptree.hpp>

namespace mapnik {
namespace formatting {

using boost::property_tree::ptree;

void layout_node::to_xml(ptree &xml) const
{
    ptree &new_node = xml.push_back(ptree::value_type("Layout", ptree()))->second;

    if (dx) set_attr(new_node, "dx", *dx);
    if (dy) set_attr(new_node, "dy", *dy);
    if (halign) set_attr(new_node, "horizontal-alignment", *halign);
    if (valign) set_attr(new_node, "vertical-alignment", *valign);
    if (jalign) set_attr(new_node, "justify-alignment", *jalign);
    if (text_ratio) set_attr(new_node, "text-ratio", *text_ratio);
    if (wrap_width) set_attr(new_node, "wrap-width", *wrap_width);
    if (wrap_before) set_attr(new_node, "wrap-before", *wrap_before);
    if (rotate_displacement) set_attr(new_node, "rotate-displacement", *rotate_displacement);
    if (orientation) set_attr(new_node, "orientation", to_expression_string(**orientation));

    if (child_) child_->to_xml(new_node);
}

node_ptr layout_node::from_xml(xml_node const& xml)
{
    std::shared_ptr<layout_node> n = std::make_shared<layout_node>();

    node_ptr child = node::from_xml(xml);
    n->set_child(child);

    n->dx = xml.get_opt_attr<double>("dx");
    n->dy = xml.get_opt_attr<double>("dy");
    n->halign = xml.get_opt_attr<horizontal_alignment_e>("horizontal-alignment");
    n->valign = xml.get_opt_attr<vertical_alignment_e>("vertical-alignment");
    n->jalign = xml.get_opt_attr<justify_alignment_e>("justify-alignment");
    n->text_ratio = xml.get_opt_attr<double>("text-ratio");
    n->wrap_width = xml.get_opt_attr<double>("wrap-width");
    n->wrap_before = xml.get_opt_attr<boolean>("wrap-before");
    n->rotate_displacement = xml.get_opt_attr<boolean>("rotate-displacement");
    n->orientation = xml.get_opt_attr<expression_ptr>("orientation");

    return n;
}

void layout_node::apply(char_properties_ptr p, feature_impl const& feature, text_layout &output) const
{
    text_layout_properties_ptr new_properties = std::make_shared<text_layout_properties>(*output.get_layout_properties());
    if (dx) new_properties->displacement.x = *dx;
    if (dy) new_properties->displacement.y = *dy;
    if (halign) new_properties->halign = *halign;
    if (valign) new_properties->valign = *valign;
    if (jalign) new_properties->jalign = *jalign;
    if (text_ratio) new_properties->text_ratio = *text_ratio;
    if (wrap_width) new_properties->wrap_width = *wrap_width;
    if (wrap_before) new_properties->wrap_before = *wrap_before;
    if (rotate_displacement) new_properties->rotate_displacement = *rotate_displacement;
    if (orientation) new_properties->orientation = *orientation;

    // starting a new offset child with the new displacement value
    text_layout_ptr child_layout = std::make_shared<text_layout>(output.get_font_manager(), output.get_scale_factor(), new_properties);
    child_layout->init_orientation(feature);

    // process contained format tree into the child node
    if (child_) {
        child_->apply(p, feature, *child_layout);
    } else {
        MAPNIK_LOG_WARN(format) << "Useless layout node: Contains no text";
    }
    output.add_child(child_layout);
}

void layout_node::set_child(node_ptr child)
{
    child_ = child;
}

node_ptr layout_node::get_child() const
{
    return child_;
}

void layout_node::add_expressions(expression_set &output) const
{
    if (child_) child_->add_expressions(output);
}

} //ns formatting
} //ns mapnik
