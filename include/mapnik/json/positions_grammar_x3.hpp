/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko

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

#ifndef MAPNIK_JSON_POSITIONS_GRAMMAR_X3_HPP
#define MAPNIK_JSON_POSITIONS_GRAMMAR_X3_HPP

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
MAPNIK_DISABLE_WARNING_POP

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/json/positions_x3.hpp>

namespace mapnik {
namespace json {

namespace grammar {

namespace x3 = boost::spirit::x3;
using positions_grammar_type = x3::rule<class positions_tag, positions>;

positions_grammar_type const positions = "Positions";

BOOST_SPIRIT_DECLARE(positions_grammar_type);

} // namespace grammar
} // namespace json
} // namespace mapnik

#endif // MAPNIK_JSON_POSITIONS_GRAMMAR_X3_HPP
