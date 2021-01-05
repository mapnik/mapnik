/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

#include <mapnik/image_options.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/include/std_pair.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace detail {

namespace qi = boost::spirit::qi;

template <typename Iterator>
struct image_options_grammar
    : qi::grammar<Iterator, image_options_map(), boost::spirit::ascii::space_type>
{
    using pair_type = std::pair<std::string, boost::optional<std::string>>;
    image_options_grammar()
        : image_options_grammar::base_type(start)
    {
        qi::lit_type lit;
        qi::char_type char_;
        start = pair >> *(lit(':') >> pair)
            ;
        pair = key >> -('=' >> value)
            ;
        key = char_("a-zA-Z_") >> *char_("a-zA-Z_0-9\\.\\-")
            ;
        value = +char_("a-zA-Z_0-9\\.\\-")
            ;
    }

    qi::rule<Iterator, image_options_map(), boost::spirit::ascii::space_type> start;
    qi::rule<Iterator, pair_type(), boost::spirit::ascii::space_type> pair;
    qi::rule<Iterator, std::string(), boost::spirit::ascii::space_type> key, value;
};

} // ns detail

image_options_map parse_image_options(std::string const& str)
{
     auto const begin = str.begin();
     auto const end = str.end();
     boost::spirit::ascii::space_type space;
     mapnik::detail::image_options_grammar<std::string::const_iterator> g;
     image_options_map options;
     bool success = boost::spirit::qi::phrase_parse(begin, end, g, space, options);
     if (!success)
     {
         throw std::runtime_error("Can't parse image options: " + str);
     }
     return options;   // RVO
}

} // ns mapnik
