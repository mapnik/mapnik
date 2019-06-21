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

#ifndef MAPNIK_TOPOJSON_GRAMMAR_X3_HPP
#define MAPNIK_TOPOJSON_GRAMMAR_X3_HPP

// mapnik
#include <mapnik/json/topology.hpp>
#include <mapnik/util/spirit_rule.hpp>

namespace mapnik { namespace topojson { namespace grammar {

MAPNIK_SPIRIT_EXTERN_RULE(start_rule, topojson::topology);

}}} // namespace mapnik::topojson::grammar

namespace mapnik { namespace json {

inline auto topojson_grammar()
{
    return topojson::grammar::start_rule;
}

}} // namespace mapnik::json

#endif //MAPNIK_TOPOJSON_GRAMMAR_X3_HPP
