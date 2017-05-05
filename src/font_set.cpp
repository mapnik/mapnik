/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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
#include <mapnik/font_set.hpp>

//stl
#include <string>

namespace mapnik
{

font_set::font_set(std::string const& name)
    : name_(name),
      face_names_() {}

// copy
font_set::font_set(font_set const& rhs)
    : name_(rhs.name_),
      face_names_(rhs.face_names_) {}

// move
font_set::font_set(font_set && rhs)
    : name_(std::move(rhs.name_)),
      face_names_(std::move(rhs.face_names_)) {}

font_set& font_set::operator=(font_set other)
{
    swap(*this, other);
    return *this;
}

void swap(font_set & lhs, font_set & rhs)
{
    using std::swap;
    swap(lhs.name_, rhs.name_);
    swap(lhs.face_names_, rhs.face_names_);
}

bool font_set::operator==(font_set const& rhs) const
{
    return name_ == rhs.name_ &&
        face_names_ == rhs.face_names_;
}

font_set::~font_set() {}

std::size_t font_set::size() const
{
    return face_names_.size();
}

void font_set::add_face_name(std::string const& face_name)
{
    face_names_.emplace_back(face_name);
}

void font_set::set_name(std::string const& name)
{
    name_ = name;
}

std::string const& font_set::get_name() const
{
    return name_;
}

std::vector<std::string> const& font_set::get_face_names() const
{
    return face_names_;
}
}
