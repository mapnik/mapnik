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

#ifndef MAPNIK_SVG_TRANSFORM_GRAMMAR_HPP
#define MAPNIK_SVG_TRANSFORM_GRAMMAR_HPP

// mapnik
#include <mapnik/global.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/qi.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace svg {

using namespace boost::spirit;

template <typename Iterator, typename TransformType, typename SkipType>
struct svg_transform_grammar : qi::grammar<Iterator, void(TransformType&), SkipType>
{
    // ctor
    svg_transform_grammar();
    // rules
    qi::rule<Iterator, void(TransformType&), SkipType> start;
    qi::rule<Iterator, void(TransformType&), SkipType> transform_;
    qi::rule<Iterator, void(TransformType&), SkipType> matrix;
    qi::rule<Iterator, void(TransformType&), SkipType> translate;
    qi::rule<Iterator, void(TransformType&), SkipType> scale;
    qi::rule<Iterator, qi::locals<double, double, double>, void(TransformType&), SkipType> rotate;
    qi::rule<Iterator, void(TransformType&), SkipType> skewX;
    qi::rule<Iterator, void(TransformType&), SkipType> skewY;
};

}}

#endif // MAPNIK_SVG_TRANSFORM_GRAMMAR_HPP
