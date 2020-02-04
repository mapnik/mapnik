/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2020 Artem Pavlenko
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

#include <mapnik/css/css_grammar_x3_def.hpp>
#include <mapnik/css/css_color_grammar_x3.hpp>

namespace mapnik { namespace css_grammar {

namespace x3 = boost::spirit::x3;
using iterator_type = char const*;
using context_type = x3::phrase_parse_context<css_skipper_type>::type;

BOOST_SPIRIT_INSTANTIATE(ident_grammar_type, iterator_type, context_type);
BOOST_SPIRIT_INSTANTIATE(css_classes_type, iterator_type, context_type);
BOOST_SPIRIT_INSTANTIATE(css_grammar_type, iterator_type, context_type);
BOOST_SPIRIT_INSTANTIATE(css_skipper_type, iterator_type, x3::unused_type);
}

css_grammar::ident_grammar_type const ident_grammar()
{
    return css_grammar::ident;
}

css_grammar::css_classes_type const classes()
{
    return css_grammar::css_classes;
}

css_grammar::css_grammar_type const grammar()
{
    return css_grammar::css_grammar;
}

css_grammar::css_skipper_type const skipper()
{
    return css_grammar::css_skipper;
}
}
