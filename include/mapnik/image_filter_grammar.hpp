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

#ifndef MAPNIK_IMAGE_FILITER_GRAMMAR_HPP
#define MAPNIK_IMAGE_FILITER_GRAMMAR_HPP

// boost
#include <boost/spirit/include/qi.hpp>

// stl
#include <vector>

namespace mapnik {

namespace qi = boost::spirit::qi;

template <typename Iterator, typename ContType>
struct image_filter_grammar :
        qi::grammar<Iterator, ContType(), qi::ascii::space_type>
{
    image_filter_grammar();
    qi::rule<Iterator, ContType(), qi::ascii::space_type> start;
    qi::rule<Iterator, ContType(), qi::locals<int,int>, qi::ascii::space_type> filter;
    qi::uint_parser< unsigned, 10, 1, 3 > radius_;
};

}

#endif // MAPNIK_IMAGE_FILITER_PARSER_HPP
