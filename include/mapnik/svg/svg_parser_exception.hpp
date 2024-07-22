/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#ifndef MAPNIK_SVG_PARSER_EXCEPTION_HPP
#define MAPNIK_SVG_PARSER_EXCEPTION_HPP

// mapnik
#include <mapnik/config.hpp>
#include <exception>

// stl
#include <map>

namespace mapnik {
namespace svg {

class MAPNIK_DECL svg_parser_exception : public std::exception
{
  public:
    svg_parser_exception(std::string const& message)
        : message_(message)
    {}

    ~svg_parser_exception() {}

    virtual const char* what() const noexcept { return message_.c_str(); }

  private:
    std::string message_;
};

} // namespace svg
} // namespace mapnik

#endif // MAPNIK_SVG_PARSER_EXCEPTION_HPP
