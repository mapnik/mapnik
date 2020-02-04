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

#include <mapnik/css/css_grammar_x3.hpp>
#include <mapnik/css/css_color_grammar_x3_def.hpp>
#if BOOST_VERSION < 107000
#include <mapnik/image_filter_types.hpp>
#endif
namespace mapnik { namespace css_color_grammar {

namespace x3 = boost::spirit::x3;
using iterator_type = std::string::const_iterator;
using iterator_css_type = char const*;
using context_type = x3::phrase_parse_context<x3::ascii::space_type>::type;
using context_css_type = x3::phrase_parse_context<css_grammar::css_skipper_type>::type;


BOOST_SPIRIT_INSTANTIATE(css_color_grammar_type, iterator_type, context_type);
BOOST_SPIRIT_INSTANTIATE(css_color_grammar_type, iterator_css_type, context_css_type);

#if BOOST_VERSION < 107000
template bool parse_rule<iterator_type, context_type, mapnik::filter::color_to_alpha>
(css_color_grammar_type, iterator_type&, iterator_type const&, context_type const&, mapnik::filter::color_to_alpha&);
#endif

}}
