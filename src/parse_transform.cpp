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

#include <mapnik/transform/parse_transform.hpp>
#include <mapnik/transform/transform_expression_grammar_x3.hpp>
#include <mapnik/expression_grammar_x3_config.hpp> // transcoder_tag
#include <mapnik/config_error.hpp>
// stl
#include <string>
#include <stdexcept>

namespace x3 = boost::spirit::x3;

namespace mapnik {

transform_list_ptr parse_transform(std::string const& str, std::string const& encoding)
{
    transform_list_ptr trans_list = std::make_shared<transform_list>();
    std::string::const_iterator itr = str.begin();
    std::string::const_iterator end = str.end();
    mapnik::transcoder const tr(encoding);
    auto const parser = x3::with<mapnik::grammar::transcoder_tag>(std::ref(tr))
        [
            mapnik::transform_expression_grammar()
        ];
    bool status = false;
    try
    {
        status = x3::phrase_parse(itr, end, parser, x3::ascii::space, *trans_list);
    }
    catch (x3::expectation_failure<std::string::const_iterator> const& ex)
    {
        throw config_error("Failed to parse transform expression: \"" + str + "\"");
    }

    if (status && itr == end)
    {
        return trans_list;
    }
    else
    {
        throw std::runtime_error("Failed to parse transform: \"" + str + "\"");
    }
}


} // namespace mapnik
