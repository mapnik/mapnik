/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2009 Artem Pavlenko
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

//$Id$

#ifndef MAPNIK_PATH_EXPRESSIONS_GRAMMAR_HPP
#define MAPNIK_PATH_EXPRESSIONS_GRAMMAR_HPP

// mapnik
#include <mapnik/unicode.hpp>
#include <mapnik/expression_node.hpp>
#include <mapnik/feature.hpp>
// boost
#include <boost/variant.hpp>
#include <boost/concept_check.hpp>

//spirit2
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_action.hpp>
//fusion
#include <boost/fusion/include/adapt_struct.hpp>
//phoenix
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/home/phoenix/object/construct.hpp>
// stl
#include <string>
#include <vector>

namespace mapnik
{

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace standard_wide =  boost::spirit::standard_wide;

//
using standard_wide::space_type;
using standard_wide::space;


template <typename Iterator>
struct path_expression_grammar : qi::grammar<Iterator, std::vector<path_component>(), space_type>
{    
    path_expression_grammar()
        : path_expression_grammar::base_type(expr)
    {
        using boost::phoenix::construct;
        using standard_wide::char_;
        using qi::_1;
        using qi::_val;
        using qi::lit;
        using qi::lexeme;
        using phoenix::push_back;
        
        expr =
            * (
                str [ push_back(_val, _1)]
                |
                ( '[' >> attr [ push_back(_val, construct<attribute>( _1 )) ] >> ']')
                )
            ;
        
        attr %= +(char_ - ']');
        str  %= lexeme[+(char_ -'[')];
    }
    
    qi::rule<Iterator, std::vector<path_component>() , space_type> expr;
    qi::rule<Iterator, std::string() , space_type> attr;
    qi::rule<Iterator, std::string() > str;
};


}

#endif  // MAPNIK_PATH_EXPRESSIONS_GRAMMAR_HPP

