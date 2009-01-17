/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id$

//mapnik
#include <mapnik/font_set.hpp>
//stl
#include <string>
#include <iostream>

namespace mapnik
{
    FontSet::FontSet()
        : name_("") {}

    FontSet::FontSet(std::string const& name)
        : name_(name) {}

    FontSet::FontSet(FontSet const& rhs)
        : name_(rhs.name_),
          face_names_(rhs.face_names_) {}
   
    FontSet& FontSet::operator=(FontSet const& other)
    {
        if (this == &other)
            return *this;
        name_ = other.name_;
        face_names_ = other.face_names_;

        return *this;
    } 

    FontSet::~FontSet() {}
    
    unsigned FontSet::size() const
    {
        return face_names_.size();
    }

    void FontSet::add_face_name(std::string face_name)
    {
        face_names_.push_back(face_name);
    }

    std::string const& FontSet::get_name() const
    {
        return name_;
    }
    
    std::vector<std::string> const& FontSet::get_face_names() const
    {
        return face_names_;
    }
}
