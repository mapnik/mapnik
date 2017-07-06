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

#ifndef MAPNIK_JSON_GENERIC_JSON_GRAMMAR_X3_HPP
#define MAPNIK_JSON_GENERIC_JSON_GRAMMAR_X3_HPP

#include <mapnik/json/json_value.hpp>
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#pragma GCC diagnostic pop

#include <vector>

namespace mapnik { namespace json {

namespace x3 = boost::spirit::x3;

namespace grammar {

using generic_json_grammar_type = x3::rule<class generic_json_tag, json_value>;
using generic_json_key_value_type = x3::rule<class json_object_element_tag, json_object_element>;
BOOST_SPIRIT_DECLARE(generic_json_grammar_type);
BOOST_SPIRIT_DECLARE(generic_json_key_value_type);
}

auto assign = [](auto const& ctx)
{
    _val(ctx) = _attr(ctx);
};

auto assign_key = [](auto const& ctx)
{
    std::get<0>(_val(ctx)) = _attr(ctx);
};

auto assign_value = [](auto const& ctx)
{
    std::get<1>(_val(ctx)) = std::move(_attr(ctx));
};

grammar::generic_json_grammar_type const& generic_json_grammar();
grammar::generic_json_key_value_type const& generic_json_key_value();

}}

#endif // MAPNIK_JSON_GENERIC_JSON_GRAMMAR_X3_HPP
