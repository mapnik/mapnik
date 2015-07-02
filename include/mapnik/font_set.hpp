/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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

#ifndef MAPNIK_FONT_SET_HPP
#define MAPNIK_FONT_SET_HPP

// mapnik
#include <mapnik/config.hpp>

// stl
#include <string>
#include <vector>

namespace mapnik
{
class MAPNIK_DECL font_set
{
public:
    // ctor/copy/move/dtor
    font_set(std::string const& name);
    font_set(font_set const& rhs);
    font_set(font_set &&);
    font_set& operator=(font_set rhs);
    ~font_set();
    // comparison
    bool operator==(font_set const& rhs) const;
    std::size_t size() const;
    void set_name(std::string const& name);
    std::string const& get_name() const;
    void add_face_name(std::string const& face_name);
    std::vector<std::string> const& get_face_names() const;

private:
    friend void swap(font_set & lhs, font_set & rhs);
    std::string name_;
    std::vector<std::string> face_names_;
};
}

#endif // MAPNIK_FONT_SET_HPP
