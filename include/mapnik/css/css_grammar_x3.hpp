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

#ifndef MAPNIK_CSS_GRAMMAR_X3_HPP
#define MAPNIK_CSS_GRAMMAR_X3_HPP

#include <vector>
#include <unordered_map>
#include <mapnik/color.hpp>
#include <mapnik/util/variant.hpp>
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#pragma GCC diagnostic pop

namespace mapnik
{
using property_value_type = mapnik::util::variant<double, mapnik::color, std::string>;
using css_key_value = std::pair<std::string, property_value_type>;
using definition_type = std::vector<css_key_value>;
using css_data = std::unordered_multimap<std::string, definition_type>;

namespace x3 = boost::spirit::x3;

namespace css_grammar
{

class css_tag;
class ident_tag;
class skipper_tag;
class classes_tag;
//
using ident_grammar_type = x3::rule<ident_tag, std::string>;
using css_grammar_type = x3::rule<css_tag, css_data>;
using css_skipper_type = x3::rule<skipper_tag, x3::unused_type const>;
using css_classes_type = x3::rule<classes_tag, std::vector<std::string>>;

ident_grammar_type const ident{"IDENT"};
css_grammar_type const css_grammar{"css"};
css_skipper_type const css_skipper{"css skipper"};
css_classes_type const css_classes{"css classes"};

BOOST_SPIRIT_DECLARE(ident_grammar_type,
                     css_classes_type,
                     css_grammar_type,
                     css_skipper_type);

}

css_grammar::ident_grammar_type const ident_grammar();
css_grammar::css_classes_type const classes();
css_grammar::css_grammar_type const grammar();
css_grammar::css_skipper_type const skipper();
}

#endif // MAPNIK_CSS_COLOR_GRAMMAR_X3_HPP
