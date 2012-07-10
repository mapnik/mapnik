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

#include <mapnik/processed_text.hpp>
#include <mapnik/config_error.hpp>

namespace mapnik
{

void processed_text::push_back(char_properties const& properties, UnicodeString const& text)
{
    expr_list_.push_back(processed_expression(properties, text));
}

void processed_text::add_child(processed_text_ptr child_text)
{
    child_text_list_.push_back(child_text);
}

processed_text::expression_list::const_iterator processed_text::begin() const
{
    return expr_list_.begin();
}

processed_text::expression_list::const_iterator processed_text::end() const
{
    return expr_list_.end();
}

processed_text_list::const_iterator processed_text::child_begin() const
{
    return child_text_list_.begin();
}

processed_text_list::const_iterator processed_text::child_end() const
{
    return child_text_list_.end();
}

processed_text::processed_text(face_manager<freetype_engine> & font_manager, double scale_factor, position displacement)
    : font_manager_(font_manager), scale_factor_(scale_factor), displacement_(displacement)
{

}

void processed_text::clear()
{
    info_.clear();
    expr_list_.clear();
}


string_info_ptr processed_text::get_string_info()
{
    info_.clear(); //if this function is called twice invalid results are returned, so clear string_info first
    expression_list::iterator itr = expr_list_.begin();
    expression_list::iterator end = expr_list_.end();
    for (; itr != end; ++itr)
    {
        char_properties const &p = itr->p;
        face_set_ptr faces = font_manager_.get_face_set(p.face_name, p.fontset);
        if (faces->size() == 0)
        {
            if (!p.fontset.get_name().empty())
            {
                throw config_error("Unable to find specified font set '" + p.fontset.get_name() + "'");
            } else if (!p.face_name.empty()) {
                throw config_error("Unable to find specified font face '" + p.face_name + "'");
            } else {
                throw config_error("Both font set and face name are empty!");
            }
        }
        faces->set_character_sizes(p.text_size * scale_factor_);
        faces->get_string_info(info_, itr->str, &(itr->p));
        info_.add_text(itr->str);
        info_.set_displacement(displacement_);
    }
    return &info_;
}

void processed_text::get_offset_info(string_info_list & offset_info_list)
{
    offset_info_list.push_back(get_string_info());
    
    processed_text_list::iterator itr = child_text_list_.begin();
    processed_text_list::iterator end = child_text_list_.end();
    for (; itr != end; ++itr)
    {
        (*itr)->get_offset_info(offset_info_list);
    }
}

} //ns mapnik
