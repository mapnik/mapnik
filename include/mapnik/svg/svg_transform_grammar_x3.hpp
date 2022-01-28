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

#ifndef MAPNIK_SVG_TRANSFORM_GRAMMAR_X3_HPP
#define MAPNIK_SVG_TRANSFORM_GRAMMAR_X3_HPP

// mapnik
#include <mapnik/svg/svg_grammar_config_x3.hpp>

namespace mapnik {
namespace svg {
namespace grammar {

using namespace boost::spirit::x3;

using svg_transform_grammar_type = x3::rule<class svg_transform_rule_tag>;

svg_transform_grammar_type const svg_transform = "SVG Transform";

BOOST_SPIRIT_DECLARE(svg_transform_grammar_type);

} // namespace grammar
} // namespace svg
} // namespace mapnik

#endif // MAPNIK_SVG_TRANSFORM_GRAMMAR_X3_HPP
