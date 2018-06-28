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

// mapnik
#include <mapnik/expression.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/unicode.hpp>
#include <mapnik/expression_node_types.hpp>
#include <mapnik/expression_grammar_x3.hpp>

namespace x3 = boost::spirit::x3;

namespace mapnik
{

expression_ptr parse_expression(std::string const& str)
{
    auto node = std::make_shared<expr_node>();
    mapnik::transcoder const tr("utf8");
    auto parser = x3::with<mapnik::grammar::transcoder_tag>(std::ref(tr))
        [
            mapnik::expression_grammar()
        ];

    bool r = false;
    std::string::const_iterator itr = str.begin();
    std::string::const_iterator const end = str.end();

    try
    {
        r = x3::phrase_parse(itr, end, parser, x3::ascii::space, *node);
    }
    catch (x3::expectation_failure<std::string::const_iterator> const& ex)
    {
        // no need to show "boost::spirit::x3::expectation_failure" which is a std::runtime_error
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
