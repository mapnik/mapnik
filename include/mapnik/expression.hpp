/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef MAPNIK_EXPRESSION_HPP
#define MAPNIK_EXPRESSION_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/expression_node.hpp>

// stl
#include <string>

namespace mapnik
{

typedef boost::shared_ptr<expr_node> expression_ptr;
template <typename Iterator> struct expression_grammar;

class expression_factory
{
public:
    static expression_ptr compile(std::string const& str,transcoder const& tr);
    static bool parse_from_string(expression_ptr const& expr,
                                  std::string const& str,
                                  mapnik::expression_grammar<std::string::const_iterator> const& g);
};

MAPNIK_DECL expression_ptr parse_expression (std::string const& wkt, std::string const& encoding);
MAPNIK_DECL expression_ptr parse_expression (std::string const& wkt);

}

#endif // MAPNIK_EXPRESSION_HPP
