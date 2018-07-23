/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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

#ifndef MAPNIK_SVG_PATH_GRAMMAR_X3_DEF_HPP
#define MAPNIK_SVG_PATH_GRAMMAR_X3_DEF_HPP

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/config.hpp>
#include <mapnik/svg/svg_path_grammar_x3.hpp>
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#pragma GCC diagnostic pop

namespace mapnik { namespace svg { namespace grammar {

namespace x3 = boost::spirit::x3;

using x3::lit;
using x3::double_;
using x3::int_;
using x3::no_case;

using coord_type = std::tuple<double,double>;

template <typename Context>
svg_converter_type & extract_path(Context const& ctx)
{
    return x3::get<svg_path_tag>(ctx);
}

template <typename Context>
bool & extract_relative(Context const& ctx)
{
    return x3::get<relative_tag>(ctx);
}

auto const move_to = [] (auto const& ctx)
{
    extract_path(ctx).move_to(std::get<0>(_attr(ctx)), std::get<1>(_attr(ctx)), x3::get<relative_tag>(ctx));
};

auto const line_to = [] (auto const & ctx)
{
    extract_path(ctx).line_to(std::get<0>(_attr(ctx)), std::get<1>(_attr(ctx)), x3::get<relative_tag>(ctx));
};

auto const hline_to = [] (auto const& ctx)
{
    extract_path(ctx).hline_to(_attr(ctx), x3::get<relative_tag>(ctx));
};

auto const vline_to = [] (auto const& ctx)
{
    extract_path(ctx).vline_to(_attr(ctx), x3::get<relative_tag>(ctx));
};

auto const curve4 = [] (auto const& ctx)
{
    auto const& attr = _attr(ctx);
    auto const& p0 = boost::fusion::at_c<0>(attr);
    auto const& p1 = boost::fusion::at_c<1>(attr);
    auto const& p2 = boost::fusion::at_c<2>(attr);
    extract_path(ctx).curve4(std::get<0>(p0),std::get<1>(p0),
                                            std::get<0>(p1),std::get<1>(p1),
                                            std::get<0>(p2),std::get<1>(p2),
                                            x3::get<relative_tag>(ctx));
};

auto const curve4_smooth = [] (auto const& ctx)
{
    auto const& attr = _attr(ctx);
    auto const& p0 = boost::fusion::at_c<0>(attr);
    auto const& p1 = boost::fusion::at_c<1>(attr);
    extract_path(ctx).curve4(std::get<0>(p0),std::get<1>(p0),
                                            std::get<0>(p1),std::get<1>(p1),
                                            x3::get<relative_tag>(ctx));
};

auto const curve3 = [] (auto const& ctx)
{
    auto const& attr = _attr(ctx);
    auto const& p0 = boost::fusion::at_c<0>(attr);
    auto const& p1 = boost::fusion::at_c<1>(attr);
    extract_path(ctx).curve3(std::get<0>(p0),std::get<1>(p0),
                                            std::get<0>(p1),std::get<1>(p1),
                                            x3::get<relative_tag>(ctx));
};

auto const curve3_smooth = [] (auto const& ctx)
{
    auto const& attr = _attr(ctx);
    extract_path(ctx).curve3(std::get<0>(attr),std::get<1>(attr),
                                            x3::get<relative_tag>(ctx));
};


auto const arc_to = [] (auto & ctx)
{
    auto const& attr = _attr(ctx);
    auto const& p = boost::fusion::at_c<0>(attr);
    double angle = boost::fusion::at_c<1>(attr);
    int large_arc_flag = boost::fusion::at_c<2>(attr);
    int sweep_flag = boost::fusion::at_c<3>(attr);
    auto const& v = boost::fusion::at_c<4>(attr);
    extract_path(ctx).arc_to(std::get<0>(p),std::get<1>(p),
                                            deg2rad(angle), large_arc_flag, sweep_flag,
                                            std::get<0>(v),std::get<1>(v),
                                            x3::get<relative_tag>(ctx));
};

auto const close_path = [] (auto const& ctx)
{
    extract_path(ctx).close_subpath();
};

auto const relative = [] (auto const& ctx)
{
    extract_relative(ctx) = true;
};

auto const absolute = [] (auto const& ctx)
{
    extract_relative(ctx) = false;
};

// exported rules
svg_path_grammar_type const svg_path = "SVG Path";
svg_points_grammar_type const svg_points = "SVG_Points";

// rules
auto const coord = x3::rule<class coord_tag, coord_type>{} = double_ > -lit(',') > double_;

auto const svg_points_def = coord[move_to] // move_to
    > *(-lit(',') >> coord[line_to]);      // *line_to

auto const M = x3::rule<class M_tag> {} = (lit('M')[absolute] | lit('m')[relative])
    > svg_points ;

auto const H = x3::rule<class H_tag> {} = (lit('H')[absolute] | lit('h')[relative])
    > (double_[ hline_to] % -lit(',')) ; // +hline_to

auto const V = x3::rule<class V_tag> {} = (lit('V')[absolute] | lit('v')[relative])
    > (double_[ vline_to] % -lit(',')) ; // +vline_to

auto const L = x3::rule<class L_tag> {} = (lit('L')[absolute] | lit('l')[relative])
    > (coord [line_to] % -lit(','));     // +line_to

auto const C = x3::rule<class C_tag> {} = (lit('C')[absolute] | lit('c')[relative])
    > ((coord > -lit(',') > coord > -lit(',') > coord)[curve4] % -lit(',')); // +curve4

auto const S = x3::rule<class S_tag> {} = (lit('S')[absolute] | lit('s')[relative])
    > ((coord > -lit(',') > coord) [curve4_smooth] % -lit(',')); // +curve4_smooth (smooth curveto)

auto const Q = x3::rule<class Q_tag> {} = (lit('Q')[absolute] | lit('q')[relative])
    > ((coord > -lit(',') > coord) [curve3] % -lit(',')); // +curve3 (quadratic-bezier-curveto)

auto const T = x3::rule<class T_tag> {} = (lit('T')[absolute] | lit('t')[relative])
    > ((coord ) [curve3_smooth] % -lit(',')); // +curve3_smooth (smooth-quadratic-bezier-curveto)

auto const A = x3::rule<class A_tag> {} = (lit('A')[absolute] | lit('a')[relative])
    > ((coord > -lit(',') > double_ > -lit(',') > int_ > -lit(',') > int_ > -lit(',') > coord)
        [arc_to] % -lit(',')); // arc_to;

auto const Z = x3::rule<class Z_tag>{} = no_case[lit('z')] [close_path]; // close path

auto const drawto_cmd = x3::rule<class drawto_cmd_tag> {} = L | H | V | C | S | Q | T | A | Z;

auto const cmd = x3::rule<class cmd_tag> {} = M > *drawto_cmd ;

auto const svg_path_def = +cmd;

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
BOOST_SPIRIT_DEFINE(
    svg_path,
    svg_points
    );
#pragma GCC diagnostic pop

}

grammar::svg_path_grammar_type const& svg_path_grammar()
{
    return grammar::svg_path;
}

grammar::svg_points_grammar_type const& svg_points_grammar()
{
    return grammar::svg_points;
}

}}

#endif // MAPNIK_SVG_PATH_GRAMMAR_X3_HPP
