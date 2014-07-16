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
#include <mapnik/text/placements/list.hpp>
#include <mapnik/xml_node.hpp>
#include <mapnik/make_unique.hpp>
//boost
#include <boost/property_tree/ptree.hpp>

namespace mapnik
{

bool text_placement_info_list::next()
{
    if (state == 0)
    {
        properties = parent_->defaults;
    }
    else
    {
        if (state  >= parent_->list_.size() + 1) return false;
        properties = parent_->list_[state-1];
        properties.layout_defaults.displacement_evaluator_ = [this](feature_impl const& feature, attributes const& attrs)
            {
                double dx = boost::apply_visitor(extract_value<value_double>(feature,attrs),
                                                 parent_->list_[state-1].layout_defaults.dx);
                double dy = boost::apply_visitor(extract_value<value_double>(feature,attrs),
                                                 parent_->list_[state-1].layout_defaults.dy);
                return pixel_position(dx,dy);
            };
    }
    ++state;
    return true;
}

text_symbolizer_properties & text_placements_list::add()
{
    if (list_.size()) {
        text_symbolizer_properties & last = list_.back();
        list_.push_back(last); //Preinitialize with old values
    } else {
        list_.push_back(defaults);
    }
    return list_.back();
}

text_symbolizer_properties & text_placements_list::get(unsigned i)
{
    return list_[i];
}


text_placement_info_ptr text_placements_list::get_placement_info(double scale_factor) const
{
    return std::make_unique<text_placement_info_list>(this, scale_factor);
}

text_placements_list::text_placements_list()
    : text_placements(),
      list_(0) {}

void text_placements_list::add_expressions(expression_set & output) const
{
    defaults.add_expressions(output);
    for (auto & prop : list_)
    {
        prop.add_expressions(output);
    }
}

unsigned text_placements_list::size() const
{
    return list_.size();
}

text_placements_ptr text_placements_list::from_xml(xml_node const& xml, fontset_map const & fontsets)
{
    using boost::property_tree::ptree;
    auto list = std::make_shared<text_placements_list>();

    list->defaults.from_xml(xml, fontsets);
    for (auto const& node : xml)
    {
        if (node.is_text() || !node.is("Placement")) continue;
        text_symbolizer_properties & p = list->add();
        p.format = std::make_shared<char_properties>(*(p.format)); //Make a deep copy <-- FIXME

        //p.layout_defaults = std::make_shared<text_layout_properties>(*(p.layout_defaults));
        //TODO: This needs a real copy constructor for text_symbolizer_properties
        p.from_xml(node, fontsets);

//TODO: if (strict_ &&
//                !p.format.fontset.size())
//            ensure_font_face(p.format.face_name);
    }
    return list;
}

} //ns mapnik
