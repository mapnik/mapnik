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

#ifndef MAPNIK_JSON_POSITIONS_GRAMMAR_X3_DEF_HPP
#define MAPNIK_JSON_POSITIONS_GRAMMAR_X3_DEF_HPP

#include <mapnik/json/positions_grammar_x3.hpp>
#include <mapnik/geometry/fusion_adapted.hpp>

namespace mapnik {
namespace json {
namespace grammar {

namespace x3 = boost::spirit::x3;
using x3::double_;
using x3::lit;
using x3::omit;

const auto assign_helper = [](auto const& ctx) {
    _val(ctx) = std::move(_attr(ctx));
};

// rules
x3::rule<class point_class, point> const point("Position");
x3::rule<class ring_class, ring> const ring("Ring");
x3::rule<class rings_class, rings> const rings("Rings");
x3::rule<class rings_array_class, rings_array> const rings_array("RingsArray");

auto const positions_def =
  rings_array[assign_helper] | rings[assign_helper] | ring[assign_helper] | point[assign_helper];
auto const point_def = lit('[') > double_ > lit(',') > double_ > omit[*(lit(',') > double_)] > lit(']');
auto const ring_def = lit('[') >> -(point % lit(',')) >> lit(']');
auto const rings_def = lit('[') >> (ring % lit(',') > lit(']'));
auto const rings_array_def = lit('[') >> (rings % lit(',') > lit(']'));

BOOST_SPIRIT_DEFINE(positions, point, ring, rings, rings_array);
} // namespace grammar
} // namespace json
} // namespace mapnik

#endif // MAPNIK_JSON_POSITIONS_GRAMMAR_X3_DEF_HPP
