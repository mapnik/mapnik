/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#ifndef MAPNIK_VALUE_ERROR_HPP
#define MAPNIK_VALUE_ERROR_HPP

#include <mapnik/config.hpp>

#include <exception>
#include <string>

namespace mapnik {

class value_error : public std::exception
{
  public:
    value_error()
        : what_()
    {}

    value_error(std::string const& what)
        : what_(what)
    {}

    virtual ~value_error() {}

    virtual const char* what() const noexcept { return what_.c_str(); }

    void append_context(std::string const& ctx) const { what_ += " " + ctx; }

  protected:
    mutable std::string what_;
};

} // namespace mapnik

#endif // MAPNIK_VALUE_ERROR_HPP
