/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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

#ifndef SVG_POINTS_GRAMMAR_HPP
#define SVG_POINTS_GRAMMAR_HPP

// mapnik
#include <mapnik/svg/svg_path_commands.hpp>
// spirit
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace mapnik { namespace svg {

    using namespace boost::spirit;
    using namespace boost::phoenix;

    template <typename Iterator, typename SkipType, typename PathType>
    struct svg_points_grammar : qi::grammar<Iterator,SkipType>
    {
        explicit svg_points_grammar(PathType & path)
            : svg_points_grammar::base_type(start),
              move_to_(move_to<PathType>(path)),
              line_to_(line_to<PathType>(path)),
              close_(close<PathType>(path))
        {
            qi::_1_type _1;
            qi::lit_type lit;
            qi::double_type double_;

            start = coord[move_to_(_1,false)] // move_to
                >> *(-lit(',') >> coord [ line_to_(_1,false) ] ); // *line_to

            coord = double_ >> -lit(',') >> double_;
        }

        // rules
        qi::rule<Iterator,SkipType> start;
        qi::rule<Iterator,boost::fusion::vector2<double,double>(),SkipType> coord;

        // commands
        function<move_to<PathType> > move_to_;
        function<line_to<PathType> > line_to_;
        function<close<PathType> > close_;
    };

    }}


#endif // SVG_POINTS_GRAMMAR_HPP
