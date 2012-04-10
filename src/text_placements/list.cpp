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
#include <mapnik/text_placements/list.hpp>
#include <mapnik/xml_node.hpp>

//boost
#include <boost/make_shared.hpp>


namespace mapnik
{

bool text_placement_info_list::next()
{
    if (state == 0) {
        properties = parent_->defaults;
    } else {
        if (state-1 >= parent_->list_.size()) return false;
        properties = parent_->list_[state-1];
    }
    state++;
    return true;
}

text_symbolizer_properties & text_placements_list::add()
{
    if (list_.size()) {
        text_symbolizer_properties &last = list_.back();
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

/***************************************************************************/

text_placement_info_ptr text_placements_list::get_placement_info(double scale_factor) const
{
    return boost::make_shared<text_placement_info_list>(this, scale_factor);
}

text_placements_list::text_placements_list() : text_placements(), list_(0)
{

}

void text_placements_list::add_expressions(expression_set &output)
{
    defaults.add_expressions(output);

    std::vector<text_symbolizer_properties>::const_iterator it;
    for (it=list_.begin(); it != list_.end(); it++)
    {
        it->add_expressions(output);
    }
}

unsigned text_placements_list::size() const
{
    return list_.size();
}

text_placements_ptr text_placements_list::from_xml(xml_node const &xml, fontset_map const & fontsets)
{
    using boost::property_tree::ptree;
    text_placements_list *list = new text_placements_list;
    text_placements_ptr ptr = text_placements_ptr(list);
    list->defaults.from_xml(xml, fontsets);
    xml_node::const_iterator itr = xml.begin();
    xml_node::const_iterator end = xml.end();
    for( ;itr != end; ++itr)
    {
        if (itr->is_text() || !itr->is("Placement")) continue;
        text_symbolizer_properties &p = list->add();
        p.from_xml(*itr, fontsets);
//TODO:        if (strict_ &&
//                !p.format.fontset.size())
//            ensure_font_face(p.format.face_name);
    }
    return ptr;
}

} //ns mapnik

