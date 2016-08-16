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

#ifndef MAPNIK_IMAGE_FILTER_GRAMMAR_HPP
#define MAPNIK_IMAGE_FILTER_GRAMMAR_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/css_color_grammar.hpp>
#include <mapnik/color.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/qi.hpp>
#pragma GCC diagnostic pop

// stl
#include <cmath>

namespace mapnik {

namespace filter {
struct color_stop;
struct colorize_alpha;
}

namespace qi = boost::spirit::qi;

struct percent_offset_impl
{
    using result_type = double;
    double operator() (double val) const
    {
        double result = std::abs(val/100.0);
        if (result > 1.0) result = 1.0;
        return result;
    }
};


template <typename Iterator, typename ContType>
struct image_filter_grammar :
        qi::grammar<Iterator, ContType(), qi::ascii::space_type>
{
    using alternative_type = qi::rule<Iterator, ContType(), qi::ascii::space_type>;

    image_filter_grammar();

    qi::rule<Iterator, ContType(), qi::ascii::space_type> start;
    qi::rule<Iterator, ContType(), qi::ascii::space_type,
                                   qi::locals<alternative_type*>> filter;
    qi::rule<Iterator, qi::ascii::space_type> no_args;
    qi::symbols<char, alternative_type*> alternatives;
    qi::uint_parser< unsigned, 10, 1, 3 > radius_;
    css_color_grammar<Iterator> css_color_;
    qi::rule<Iterator, filter::color_stop(), qi::ascii::space_type> color_stop_;
    qi::rule<Iterator, double(), qi::ascii::space_type> color_stop_offset;

private:
    alternative_type & add(std::string const& symbol);
    static constexpr unsigned max_alternatives = 16;
    unsigned num_alternatives = 0;
    alternative_type alternative_storage[max_alternatives];
};

} // namespace mapnik

#endif // MAPNIK_IMAGE_FILTER_GRAMMAR_HPP
