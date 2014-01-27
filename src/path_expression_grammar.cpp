/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

// mapnik
#include <mapnik/path_expression_grammar.hpp>
#include <mapnik/attribute.hpp>

// boost
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

namespace mapnik
{

template <typename Iterator>
path_expression_grammar<Iterator>::path_expression_grammar()
    : path_expression_grammar::base_type(expr)
{
    standard_wide::char_type char_;
    qi::_1_type _1;
    qi::_val_type _val;
    qi::lit_type lit;
    qi::lexeme_type lexeme;
    using phoenix::push_back;
    using boost::phoenix::construct;
    
    expr =
        * (
            str [ push_back(_val, _1)]
            |
            ( '[' >> attr [ push_back(_val, construct<mapnik::attribute>( _1 )) ] >> ']')
            )
        ;
    
    attr %= +(char_ - ']');
    str  %= lexeme[+(char_ -'[')];
}

template struct mapnik::path_expression_grammar<std::string::const_iterator>;

}
