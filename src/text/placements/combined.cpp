/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Walid Ibrahim
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
#include <mapnik/text/placements/combined.hpp>
#include <mapnik/xml_node.hpp>

#include <mapnik/text/placements/list.hpp>
#include <mapnik/text/placements/simple.hpp>

namespace mapnik
{

//text_placements_combined class
text_placements_combined::text_placements_combined(text_placements_ptr simple_placement, text_placements_ptr list_placement)
: simple_placement_(simple_placement),
  list_placement_(list_placement)
{
}

text_placement_info_ptr text_placements_combined::get_placement_info(double scale_factor, feature_impl const& feature, attributes const& vars) const
{
    text_placement_info_ptr simple_placement_info = simple_placement_->get_placement_info(scale_factor, feature, vars);
    
    text_placement_info_ptr list_placement_info = list_placement_->get_placement_info(scale_factor, feature, vars);
    
    return std::make_shared<text_placement_info_combined>(this, scale_factor, simple_placement_info, list_placement_info);
}
    
void text_placements_combined::add_expressions(expression_set & output) const
{
    simple_placement_->add_expressions(output);
    list_placement_->add_expressions(output);
}

text_placements_ptr text_placements_combined::from_xml(xml_node const& xml, fontset_map const& fontsets, bool is_shield)
{
    text_placements_ptr simple_placement_ptr = text_placements_simple::from_xml(xml, fontsets, is_shield);
    text_placements_ptr list_placement_ptr = text_placements_list::from_xml(xml, fontsets, is_shield);
    if(simple_placement_ptr && list_placement_ptr)
    {
        return std::make_shared<text_placements_combined>(simple_placement_ptr, list_placement_ptr);
    }
    return text_placements_ptr();
}
    
//text_placement_info_combined class
    
text_placement_info_combined::text_placement_info_combined(text_placements_combined const* parent, double scale_factor, text_placement_info_ptr simple_placement_info, text_placement_info_ptr list_placement_info)
: text_placement_info(parent, scale_factor),
  parent_(parent),
  simple_placement_info_(simple_placement_info),
  list_placement_info_(list_placement_info)
{
}
    
void text_placement_info_combined::reset_state()
{
    simple_placement_info_->reset_state();
    list_placement_info_->reset_state();
}
    
bool text_placement_info_combined::next() const
{
    //logic to get the next combined point placement
    if(simple_placement_info_ && list_placement_info_)
    {
        if(simple_placement_info_->next())
        {
            properties.format_defaults.text_size = simple_placement_info_->properties.format_defaults.text_size;
            properties.layout_defaults.dir = simple_placement_info_->properties.layout_defaults.dir;
        }
        else if(list_placement_info_->next())
        {
            //simple reset state
            simple_placement_info_->reset_state();
            simple_placement_info_->properties = list_placement_info_->properties;
            properties = list_placement_info_->properties;
            if(simple_placement_info_->next())
            {
                properties.format_defaults.text_size = simple_placement_info_->properties.format_defaults.text_size;
                properties.layout_defaults.dir = simple_placement_info_->properties.layout_defaults.dir;
            }
        }
        else
        {
            return false;
        }
         return true;
    }
    return false;
}
    
}//namespace