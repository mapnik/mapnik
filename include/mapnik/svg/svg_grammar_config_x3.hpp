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

#ifndef MAPNIK_SVG_GRAMMAR_CONFIG_X3_HPP
#define MAPNIK_SVG_GRAMMAR_CONFIG_X3_HPP

#include <mapnik/svg/svg_path_parser.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
MAPNIK_DISABLE_WARNING_POP

namespace mapnik { namespace svg { namespace grammar {

class relative_tag;
class svg_path_tag;
class svg_transform_tag;

namespace x3 = boost::spirit::x3;
using space_type = x3::standard::space_type;
using iterator_type = char const*;

#if BOOST_VERSION >= 106700
using svg_converter_wrapper_type = svg_converter_type;
using relative_type = bool;
#else
using svg_converter_wrapper_type = std::reference_wrapper<svg_converter_type> const;
using relative_type = std::reference_wrapper<bool> const;
#endif

using phrase_parse_context_type = x3::phrase_parse_context<space_type>::type;
using svg_parse_context_type = x3::context<relative_tag, relative_type,
                                           x3::context<svg_path_tag, svg_converter_wrapper_type,
                                                       phrase_parse_context_type>>;

}}}

#endif // MAPNIK_SVG_GRAMMAR_CONFIG_X3_HPP
