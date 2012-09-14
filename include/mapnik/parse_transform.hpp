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

#ifndef MAPNIK_PARSE_TRANSFORM_HPP
#define MAPNIK_PARSE_TRANSFORM_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/transform_expression.hpp>

namespace mapnik {

template <typename Iterator> struct transform_expression_grammar;

typedef transform_expression_grammar<std::string::const_iterator>
    transform_expression_grammar__string;

MAPNIK_DECL transform_list_ptr parse_transform(std::string const& str);

MAPNIK_DECL transform_list_ptr parse_transform(std::string const& str,
                                               std::string const& encoding);

MAPNIK_DECL bool parse_transform(transform_list& list,
                                 std::string const& str,
                                 transform_expression_grammar__string const& g);

} // namespace mapnik

#endif // MAPNIK_PARSE_TRANSFORM_HPP
