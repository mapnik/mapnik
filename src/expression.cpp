/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/expression.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/expression_node_types.hpp>
#include <mapnik/expression_grammar.hpp>

// boost
#include <boost/spirit/include/qi.hpp>

namespace mapnik
{

expression_ptr parse_expression(std::string const& str)
{
    static const expression_grammar<std::string::const_iterator> g;
    boost::spirit::standard_wide::space_type space;
    auto node = std::make_shared<expr_node>();
    std::string::const_iterator itr = str.begin();
    std::string::const_iterator end = str.end();
    bool r = false;
    try
    {
        r = boost::spirit::qi::phrase_parse(itr, end, g, space, *node);
    }
    catch (boost::spirit::qi::expectation_failure<std::string::const_iterator> const& ex)
    {
        // no need to show "boost::spirit::qi::expectation_failure" which is a std::runtime_error
        throw config_error("Failed to parse expression: \"" + str + "\"");
    }
    catch (std::exception const& ex)
    {
        // show "Could not initialize ICU resources" from boost::regex which is a std::runtime_error
        throw config_error(std::string(ex.what()) + " for expression: \"" + str + "\"");
    }
    if (r && itr == end)
    {
        return node;
    }
    else
    {
        throw config_error("Failed to parse expression: \"" + str + "\"");
    }
}


}
