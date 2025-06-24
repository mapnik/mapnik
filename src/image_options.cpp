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

#include <mapnik/image_options.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/std_pair.hpp>
MAPNIK_DISABLE_WARNING_POP

#if defined(HAVE_PNG)
extern "C" {
#include <png.h>
}
#endif // HAVE_PNG

namespace mapnik {
namespace grammar {

namespace x3 = boost::spirit::x3;

using x3::lit;
using x3::ascii::char_;
using pair_type = std::pair<std::string, std::optional<std::string>>;

x3::rule<class image_options, image_options_map> const image_options("image options");
x3::rule<class key_value, pair_type> const key_value("key_value");
x3::rule<class key, std::string> const key("key");
x3::rule<class value, std::string> const value("value");

auto const key_def = char_("a-zA-Z_") > *char_("a-zA-Z_0-9\\.\\-");
auto const value_def = +char_("a-zA-Z_0-9\\.\\-|");
auto const key_value_def = key > -('=' > value);
auto const image_options_def = key_value % lit(':');

BOOST_SPIRIT_DEFINE(key);
BOOST_SPIRIT_DEFINE(value);
BOOST_SPIRIT_DEFINE(key_value);
BOOST_SPIRIT_DEFINE(image_options);

} // namespace grammar

image_options_map parse_image_options(std::string const& str)
{
    auto begin = str.begin();
    auto const end = str.end();
    using boost::spirit::x3::space;
    using mapnik::grammar::image_options;
    image_options_map options;
    try
    {
        bool success = boost::spirit::x3::phrase_parse(begin, end, image_options, space, options);
        if (!success || begin != end)
        {
            throw std::runtime_error("Can't parse image options: " + str);
        }
    }
    catch (boost::spirit::x3::expectation_failure<std::string> const& ex)
    {
        throw std::runtime_error("Can't parse image options: " + str + " " + ex.what());
    }
    return options; // RVO
}

#if defined(HAVE_PNG)

int parse_png_filters(std::string const& str)
{
    auto begin = str.begin();
    auto const end = str.end();
    using boost::spirit::x3::space;
    using boost::spirit::x3::symbols;
    symbols<int> filter;
    filter.add("none", PNG_FILTER_NONE)("sub", PNG_FILTER_SUB)("up", PNG_FILTER_UP)("avg",
                                                                                    PNG_FILTER_AVG)("paeth",
                                                                                                    PNG_FILTER_PAETH);

    std::vector<int> opts;
    try
    {
        bool success = boost::spirit::x3::phrase_parse(begin, end, filter % "|", space, opts);
        if (!success || begin != end)
        {
            throw std::runtime_error("Can't parse PNG filters: " + str);
        }
    }
    catch (boost::spirit::x3::expectation_failure<std::string> const& ex)
    {
        throw std::runtime_error("Can't parse PNG filters: " + str + " " + ex.what());
    }
    int filters = 0;
    std::for_each(opts.begin(), opts.end(), [&filters](int f) { filters |= f; });
    return filters;
}

#endif // HAVE_PNG

} // namespace mapnik
