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

#ifndef MAPNIK_WKT_GRAMMAR_X3_HPP
#define MAPNIK_WKT_GRAMMAR_X3_HPP

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#pragma GCC diagnostic pop

#include <mapnik/geometry.hpp>

namespace mapnik { namespace grammar {

namespace x3 = boost::spirit::x3;
struct wkt_class; // top-most ID
using wkt_grammar_type = x3::rule<wkt_class, mapnik::geometry::geometry<double>>;

wkt_grammar_type const wkt("wkt");

BOOST_SPIRIT_DECLARE(wkt_grammar_type);

}}


#endif // MAPNIK_WKT_GRAMMAR_X3_HPP
