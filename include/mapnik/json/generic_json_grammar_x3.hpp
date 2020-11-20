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
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
MAPNIK_DISABLE_WARNING_POP

#include <vector>

namespace mapnik { namespace json {

namespace x3 = boost::spirit::x3;

namespace grammar {

using generic_json_grammar_type = x3::rule<class generic_json_tag, json_value>;
using generic_json_key_value_type = x3::rule<class json_object_element_tag, json_object_element>;

generic_json_grammar_type const value = "JSON Value";
generic_json_key_value_type const key_value = "JSON Object element";

BOOST_SPIRIT_DECLARE(generic_json_grammar_type);
BOOST_SPIRIT_DECLARE(generic_json_key_value_type);

}}}

#endif // MAPNIK_JSON_GENERIC_JSON_GRAMMAR_X3_HPP
