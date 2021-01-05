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

// NOTE: This is an implementation header file and is only meant to be included
//    from implementation files. It therefore doesn't have an include guard.
// mapnik
#include <mapnik/svg/svg_points_grammar.hpp>
#include <mapnik/svg/svg_path_commands.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace svg {

using namespace boost::spirit;
using namespace boost::phoenix;

template <typename Iterator, typename PathType, typename SkipType>
svg_points_grammar<Iterator, PathType,SkipType>::svg_points_grammar()
    : svg_points_grammar::base_type(start)
{
    qi::_1_type _1;
    qi::_r1_type _r1;
    qi::lit_type lit;
    qi::double_type double_;
    // commands
    function<move_to> move_to_;
    function<line_to> line_to_;

    start = coord[move_to_(_r1, _1, false)] // move_to
        >> *(-lit(',') >> coord [ line_to_(_r1, _1,false) ] ); // *line_to

    coord = double_ >> -lit(',') >> double_;
}

}}
