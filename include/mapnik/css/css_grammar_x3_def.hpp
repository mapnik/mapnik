/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2024 Artem Pavlenko
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

#ifndef MAPNIK_CSS_GRAMMAR_X3_DEF_HPP
#define MAPNIK_CSS_GRAMMAR_X3_DEF_HPP

#include <mapnik/css/css_grammar_x3.hpp>
#include <mapnik/json/unicode_string_grammar_x3.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
MAPNIK_DISABLE_WARNING_POP

/*
The grammar below is LL(1) (but note that most UA's should not use it directly, since it doesn't express the parsing
conventions, only the CSS1 syntax). The format of the productions is optimized for human consumption and some shorthand
notation beyond yacc [15] is used:

*  : 0 or more
+  : 1 or more
?  : 0 or 1
|  : separates alternatives
[] : grouping

The productions are:

stylesheet
 : [CDO|CDC]* [ import [CDO|CDC]* ]* [ ruleset [CDO|CDC]* ]*
 ;
import
 : IMPORT_SYM [STRING|URL] ';'		// E.g., @import url(fun.css);
 ;
unary_operator
 : '-' | '+'
 ;
operator
 : '/' | ',' | // empty
 ;
property
 : IDENT
 ;
ruleset
 : selector [ ',' selector ]*
   '{' declaration [ ';' declaration ]* '}'
 ;
selector
 : simple_selector+ [ pseudo_element | solitary_pseudo_element ]?
 | solitary_pseudo_element
 ;
        // An "id" is an ID that is attached to an element type
        // on its left, as in: P#p007
        // A "solitary_id" is an ID that is not so attached,
        / as in: #p007
        // Analogously for classes and pseudo-classes.

simple_selector
: element_name id? class? pseudo_class?	// eg: H1.subject
| solitary_id class? pseudo_class?		// eg: #xyz33
 | solitary_class pseudo_class?			// eg: .author
 | solitary_pseudo_class			// eg: :link
 ;
element_name
 : IDENT
 ;
pseudo_class					// as in:  A:link
 : LINK_PSCLASS_AFTER_IDENT
 | VISITED_PSCLASS_AFTER_IDENT
 | ACTIVE_PSCLASS_AFTER_IDENT
 ;
solitary_pseudo_class				// as in:  :link
 : LINK_PSCLASS
 | VISITED_PSCLASS
 | ACTIVE_PSCLASS
 ;
class						// as in:  P.note
 : CLASS_AFTER_IDENT
 ;
solitary_class					// as in:  .note
 : CLASS
 ;
pseudo_element					// as in:  P:first-line
 : FIRST_LETTER_AFTER_IDENT
 | FIRST_LINE_AFTER_IDENT
 ;
solitary_pseudo_element				// as in:  :first-line
 : FIRST_LETTER
 | FIRST_LINE
 ;
        // There is a constraint on the id and solitary_id that the
        // part after the "#" must be a valid HTML ID value;
        // e.g., "#x77" is OK, but "#77" is not.

id
 : HASH_AFTER_IDENT
 ;
solitary_id
 : HASH
 ;
declaration
 : property ':' expr prio?
 | // empty                             // Prevents syntax errors...
 ;
prio
 : IMPORTANT_SYM                        // !important
 ;
expr
 : term [ operator term ]*
 ;
term
 : unary_operator?
   [ NUMBER | STRING | PERCENTAGE | LENGTH | EMS | EXS
   | IDENT | hexcolor | URL | RGB ]
 ;
        // There is a constraint on the color that it must
        // have either 3 or 6 hex-digits (i.e., [0-9a-fA-F])
        // after the "#"; e.g., "#000" is OK, but "#abcd" is not.

hexcolor
 : HASH | HASH_AFTER_IDENT
 ;

*/

namespace mapnik {

namespace x3 = boost::spirit::x3;

namespace css_grammar {

using x3::alnum;
using x3::alpha;
using x3::char_;
using x3::lexeme;
using x3::lit;
using x3::raw;
using x3::standard::space;

// import unicode string rule
const auto css_string = mapnik::json::grammar::unicode_string;

const auto assign_def = [](auto const& ctx) {
    for (auto const& k : std::get<0>(_attr(ctx)))
    {
        _val(ctx).emplace(k, std::get<1>(_attr(ctx)));
    }
};

const auto assign_key = [](auto const& ctx) {
    _val(ctx).first = std::move(_attr(ctx));
};

const auto assign_value = [](auto const& ctx) {
    _val(ctx).second = std::move(_attr(ctx));
};

// rules
x3::rule<class simple_selector_tag, std::string> const simple_selector{"Simple selector"};
x3::rule<class selector_tag, std::vector<std::string>> const selector{"Selector"};
x3::rule<class value_tag, property_value_type> const value{"Value"};
x3::rule<class key_value_tag, css_key_value> const key_value{"CSS Key/Value"};
x3::rule<class definition_tag, std::pair<std::vector<std::string>, definition_type>> const definition{"CSS Definition"};

auto const ident_def = alpha >> *(alnum | char_('-'));
auto const simple_selector_def = lexeme[ident >> -((char_('.') | char_('#') | char_(':')) >> ident)] |
                                 lexeme[char_('#') >> ident >> -((char_('.') | char_(':')) >> ident)] |
                                 lexeme[char_('.') >> ident >> -(char_(':') >> ident)];

auto const selector_def = simple_selector % lit(',');
auto const value_def = raw[lexeme[+~char_(";}")]];
auto const key_value_def = lexeme[ident][assign_key] >> lit(':') >> value[assign_value] >> -lit(';');
auto const definition_def = selector >> lit('{') >> *key_value >> lit('}');

auto const css_grammar_def = *definition[assign_def];
auto const css_skipper_def = space | "/*" >> *(char_ - "*/") >> "*/";

auto const css_classes_def = +lexeme[ident];

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
BOOST_SPIRIT_DEFINE(ident,
                    css_classes,
                    simple_selector,
                    selector,
                    value,
                    key_value,
                    definition,
                    css_grammar,
                    css_skipper);

MAPNIK_DISABLE_WARNING_POP

} // namespace css_grammar
} // namespace mapnik

#endif // MAPNIK_CSS_GRAMMAR_X3_DEF_HPP
