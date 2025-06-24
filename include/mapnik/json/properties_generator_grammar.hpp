/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

#ifndef MAPNIK_JSON_PROPERTIES_GENERATOR_GRAMMAR_HPP
#define MAPNIK_JSON_PROPERTIES_GENERATOR_GRAMMAR_HPP

#include <mapnik/value/types.hpp>
#include <mapnik/value.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/karma.hpp>
#include <boost/phoenix/function.hpp>
MAPNIK_DISABLE_WARNING_POP

#include <string>
#include <tuple>

namespace mapnik {
namespace json {

namespace karma = boost::spirit::karma;

template<typename OutputIterator>
struct escaped_string : karma::grammar<OutputIterator, std::string(char const*)>
{
    escaped_string();
    karma::rule<OutputIterator, std::string(char const*)> esc_str;
    karma::symbols<char, char const*> esc_char;
};

struct extract_string
{
    using result_type = std::tuple<std::string, bool>;

    result_type operator()(mapnik::value const& val) const
    {
        bool need_quotes = val.is<value_unicode_string>();
        return std::make_tuple(val.to_string(), need_quotes);
    }
};

template<typename OutputIterator, typename KeyValueStore>
struct properties_generator_grammar : karma::grammar<OutputIterator, KeyValueStore()>
{
    using pair_type = std::tuple<std::string, mapnik::value>;
    properties_generator_grammar();
    // rules
    karma::rule<OutputIterator, KeyValueStore()> properties;
    karma::rule<OutputIterator, pair_type()> pair;
    karma::rule<OutputIterator, std::tuple<std::string, bool>()> value;
    //
    escaped_string<OutputIterator> escaped_string_;
    boost::phoenix::function<extract_string> extract_string_;
    std::string quote_;
};

} // namespace json
} // namespace mapnik

#endif // MAPNIK_JSON_PROPERTIES_GENERATOR_GRAMMAR_HPP
