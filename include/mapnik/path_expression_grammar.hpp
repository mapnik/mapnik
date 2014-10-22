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

#ifndef MAPNIK_PATH_EXPRESSIONS_GRAMMAR_HPP
#define MAPNIK_PATH_EXPRESSIONS_GRAMMAR_HPP

// mapnik
#include <mapnik/path_expression.hpp>

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#include <boost/spirit/include/qi.hpp>
#pragma GCC diagnostic pop

// stl
#include <string>
#include <vector>

namespace mapnik
{

using namespace boost;
namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace standard_wide =  boost::spirit::standard_wide;

using standard_wide::space_type;

template <typename Iterator>
struct path_expression_grammar : qi::grammar<Iterator, std::vector<path_component>(), space_type>
{
    path_expression_grammar();
    qi::rule<Iterator, std::vector<path_component>() , space_type> expr;
    qi::rule<Iterator, std::string() , space_type> attr;
    qi::rule<Iterator, std::string() > str;
};

}

#endif  // MAPNIK_PATH_EXPRESSIONS_GRAMMAR_HPP
