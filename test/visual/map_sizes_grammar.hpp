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

#ifndef MAP_SIZES_GRAMMAR_HPP
#define MAP_SIZES_GRAMMAR_HPP

// boost
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#pragma GCC diagnostic pop

namespace visual_tests
{

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

template <typename Iterator>
struct map_sizes_grammar : qi::grammar<Iterator, std::vector<map_size>(), ascii::space_type>
{
    map_sizes_grammar() : map_sizes_grammar::base_type(start)
    {
        using namespace boost::spirit::qi;
        using namespace boost::phoenix;

        int_type int_;
        _1_type _1;
        _2_type _2;
        _val_type _val;

        start = (int_ >> ',' >> int_)[push_back(_val, construct<map_size>(_1, _2))] % ';';
    }

    qi::rule<Iterator, std::vector<map_size>(), ascii::space_type> start;
};

}

#endif
