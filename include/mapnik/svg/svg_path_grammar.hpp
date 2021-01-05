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

#ifndef MAPNIK_SVG_PATH_GRAMMAR_HPP
#define MAPNIK_SVG_PATH_GRAMMAR_HPP

// spirit
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/qi.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace svg {

using namespace boost::spirit;

template <typename Iterator, typename PathType, typename SkipType>
struct svg_path_grammar : qi::grammar<Iterator, void(PathType&), SkipType>
{
    // ctor
    svg_path_grammar();
    // rules
    qi::rule<Iterator, void(PathType&), SkipType> start;
    qi::rule<Iterator, void(PathType&), SkipType> cmd;
    qi::rule<Iterator, void(PathType&), SkipType> drawto_cmd;
    qi::rule<Iterator, qi::locals<bool>, void(PathType&), SkipType> M; // M,m
    qi::rule<Iterator, qi::locals<bool>, void(PathType&), SkipType> L; // L,l
    qi::rule<Iterator, qi::locals<bool>, void(PathType&), SkipType> H; // H,h
    qi::rule<Iterator, qi::locals<bool>, void(PathType&), SkipType> V; // V,v
    qi::rule<Iterator, qi::locals<bool>, void(PathType&), SkipType> C; // C,c
    qi::rule<Iterator, qi::locals<bool>, void(PathType&), SkipType> S; // S,s
    qi::rule<Iterator, qi::locals<bool>, void(PathType&), SkipType> Q; // Q,q
    qi::rule<Iterator, qi::locals<bool>, void(PathType&), SkipType> T; // T,t
    qi::rule<Iterator, qi::locals<bool>, void(PathType&), SkipType> A; // A,a
    qi::rule<Iterator, void(PathType&), SkipType> Z;                   // Z,z
    qi::rule<Iterator, boost::fusion::vector2<double, double>(), SkipType> coord;
};

}}


#endif // MAPNIK_SVG_PATH_GRAMMAR_HPP
