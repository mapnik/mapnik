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
#include <mapnik/svg/svg_path_grammar.hpp>
#include <mapnik/svg/svg_path_commands.hpp>

// spirit
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace svg {

using namespace boost::spirit;
using namespace boost::phoenix;


template <typename Iterator, typename PathType, typename SkipType>
svg_path_grammar<Iterator, PathType, SkipType>::svg_path_grammar()
    : svg_path_grammar::base_type(start)
{
    qi::_1_type _1;
    qi::_2_type _2;
    qi::_3_type _3;
    qi::_4_type _4;
    qi::_5_type _5;
    qi::_a_type _a;
    qi::lit_type lit;
    qi::_r1_type _r1;
    qi::double_type double_;
    qi::int_type int_;
    qi::no_case_type no_case;

    // commands
    function<move_to> move_to_;
    function<hline_to> hline_to_;
    function<vline_to> vline_to_;
    function<line_to> line_to_;
    function<curve4> curve4_;
    function<curve4_smooth> curve4_smooth_;
    function<curve3> curve3_;
    function<curve3_smooth> curve3_smooth_;
    function<arc_to> arc_to_;
    function<close> close_;
    //
    start = +cmd(_r1);

    cmd = M(_r1) >> *drawto_cmd(_r1);

    drawto_cmd = L(_r1) | H(_r1) | V(_r1) | C(_r1) | S(_r1) | Q(_r1) | T(_r1) | A(_r1) | Z(_r1);

    M = (lit('M')[_a = false] | lit('m')[_a = true]) >> coord[move_to_(_r1, _1, _a)] // move_to
        >> *(-lit(',') >> coord[line_to_(_r1, _1, _a)]);                             // *line_to

    H = (lit('H')[_a = false] | lit('h')[_a = true])
        >> (double_[ hline_to_(_r1, _1,_a) ] % -lit(',')) ; // +hline_to

    V = (lit('V')[_a = false] | lit('v')[_a = true])
        >> (double_ [ vline_to_(_r1, _1,_a) ] % -lit(',')); // +vline_to

    L = (lit('L')[_a = false] | lit('l')[_a = true])
        >> (coord [ line_to_(_r1, _1, _a) ] % -lit(',')); // +line_to

    C = (lit('C')[_a = false] | lit('c')[_a = true])
        >> ((coord >> -lit(',') >> coord >> -lit(',') >> coord)[curve4_(_r1, _1, _2, _3, _a)] % -lit(',')); // +curve4

    S = (lit('S')[_a = false] | lit('s')[_a = true])
        >> ((coord >> -lit(',') >> coord) [ curve4_smooth_(_r1, _1,_2,_a) ] % -lit(',')); // +curve4_smooth (smooth curveto)

    Q = (lit('Q')[_a = false] | lit('q')[_a = true])
        >> ((coord >> -lit(',') >> coord) [ curve3_(_r1, _1,_2,_a) ] % -lit(',')); // +curve3 (quadratic-bezier-curveto)

    T = (lit('T')[_a = false] | lit('t')[_a = true])
        >> ((coord ) [ curve3_smooth_(_r1, _1,_a) ] % -lit(',')); // +curve3_smooth (smooth-quadratic-bezier-curveto)

    A = (lit('A')[_a = false] | lit('a')[_a = true])
        >> ((coord >> -lit(',') >> double_ >> -lit(',') >> int_ >> -lit(',') >> int_ >> -lit(',') >> coord)
            [arc_to_(_r1, _1, _2, _3, _4, _5, _a)] % -lit(',')); // arc_to;

    Z = no_case[lit('z')] [close_(_r1)]; // close path

    coord = double_ >> -lit(',') >> double_;
}

} // namespace svg
} // namespace mapnik
