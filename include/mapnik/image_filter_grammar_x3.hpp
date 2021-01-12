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

#ifndef MAPNIK_IMAGE_FILTER_GRAMMAR_X3_HPP
#define MAPNIK_IMAGE_FILTER_GRAMMAR_X3_HPP

#include <mapnik/image_filter_types.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik
{

namespace x3 = boost::spirit::x3;

namespace image_filter
{

struct image_filter_class;
using image_filter_grammar_type = x3::rule<image_filter_class, std::vector<filter::filter_type> >;

image_filter_grammar_type const start = "start";

BOOST_SPIRIT_DECLARE(image_filter_grammar_type);

}}

#endif // MAPNIK_IMAGE_FILTER_GRAMMAR_X3_HPP
