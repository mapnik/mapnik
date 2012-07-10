/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

#include <mapnik/parse_transform.hpp>
#include <mapnik/transform_expression_grammar.hpp>
#include <mapnik/transform_processor.hpp>
#include <mapnik/debug.hpp>

#include <boost/make_shared.hpp>

namespace mapnik {

transform_list_ptr parse_transform(std::string const& str)
{
    return parse_transform(str, "utf-8");
}

transform_list_ptr parse_transform(std::string const& str, std::string const& encoding)
{
    transform_list_ptr tl = boost::make_shared<transform_list>();
    transcoder tc(encoding);
    expression_grammar<std::string::const_iterator> ge(tc);
    transform_expression_grammar__string gte(ge);

    if (!parse_transform(*tl, str, gte))
    {
        tl.reset();
    }
    return tl;
}

bool parse_transform(transform_list& transform,
                     std::string const& str,
                     transform_expression_grammar__string const& g)
{
    std::string::const_iterator itr = str.begin();
    std::string::const_iterator end = str.end();
    bool r = qi::phrase_parse(itr, end, g, space_type(), transform);

    #ifdef MAPNIK_LOG
    MAPNIK_LOG_DEBUG(load_map) << "map_parser: Parsed transform [ "
        << transform_processor_type::to_string(transform) << " ]";
    #endif

    return (r && itr==end);
}

} // namespace mapnik
