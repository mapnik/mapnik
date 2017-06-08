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

#ifndef MAPNIK_SVG_TRANSFORM_GRAMMAR_X3_DEF_HPP
#define MAPNIK_SVG_TRANSFORM_GRAMMAR_X3_DEF_HPP

// mapnik
#include <mapnik/svg/svg_transform_grammar_x3.hpp>
// boost::fusion
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/fusion/sequence.hpp>
#pragma GCC diagnostic pop
// agg
#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include <agg_trans_affine.h>
#pragma GCC diagnostic pop

namespace mapnik { namespace svg { namespace grammar {

namespace x3 = boost::spirit::x3;

using x3::lit;
using x3::double_;
using x3::no_case;

auto const matrix_action = [] (auto const& ctx)
{
    auto & tr = x3::get<svg_transform_tag>(ctx).get();
    auto const& attr = _attr(ctx);
    auto a = boost::fusion::at_c<0>(attr);
    auto b = boost::fusion::at_c<1>(attr);
    auto c = boost::fusion::at_c<2>(attr);
    auto d = boost::fusion::at_c<3>(attr);
    auto e = boost::fusion::at_c<4>(attr);
    auto f = boost::fusion::at_c<5>(attr);
    tr = agg::trans_affine(a, b, c, d, e, f) * tr;
};

auto const rotate_action = [] (auto const& ctx)
{
    auto & tr = x3::get<svg_transform_tag>(ctx).get();
    auto const& attr = _attr(ctx);
    auto a = boost::fusion::at_c<0>(attr);
    auto cx = boost::fusion::at_c<1>(attr) ? *boost::fusion::at_c<1>(attr) : 0.0;
    auto cy = boost::fusion::at_c<2>(attr) ? *boost::fusion::at_c<2>(attr) : 0.0;
    if (cx == 0.0 && cy == 0.0)
    {
        tr = agg::trans_affine_rotation(deg2rad(a)) * tr;
    }
    else
    {
        agg::trans_affine t = agg::trans_affine_translation(-cx, -cy);
        t *= agg::trans_affine_rotation(deg2rad(a));
        t *= agg::trans_affine_translation(cx, cy);
        tr = t * tr;
    }
};

auto const translate_action = [] (auto const& ctx)
{
    auto & tr = x3::get<svg_transform_tag>(ctx).get();
    auto const& attr = _attr(ctx);
    auto tx = boost::fusion::at_c<0>(attr);
    auto ty = boost::fusion::at_c<1>(attr);
    if (ty) tr = agg::trans_affine_translation(tx, *ty) * tr;
    else tr = agg::trans_affine_translation(tx,0.0) * tr;
};

auto const scale_action = [] (auto const& ctx)
{
    auto & tr = x3::get<svg_transform_tag>(ctx).get();
    auto const& attr = _attr(ctx);
    auto sx = boost::fusion::at_c<0>(attr);
    auto sy = boost::fusion::at_c<1>(attr);
    if (sy) tr = agg::trans_affine_scaling(sx, *sy) * tr;
    else tr = agg::trans_affine_scaling(sx, sx) * tr;
};

auto const skewX_action = [] (auto const& ctx)
{
    auto & tr = x3::get<svg_transform_tag>(ctx).get();
    auto skew_x = _attr(ctx);
    tr = agg::trans_affine_skewing(deg2rad(skew_x), 0.0) * tr;
};

auto const skewY_action = [] (auto const& ctx)
{
    auto & tr = x3::get<svg_transform_tag>(ctx).get();
    auto skew_y= _attr(ctx);
    tr = agg::trans_affine_skewing(0.0, deg2rad(skew_y)) * tr;
};

//exported rule
svg_transform_grammar_type const svg_transform = "SVG Transform";
// rules
auto const matrix = x3::rule<class matrix_tag> {} = no_case[lit("matrix")]
    > lit('(') > (double_ > -lit(',')
                  > double_ > -lit(',')
                  > double_ > -lit(',')
                  > double_ > -lit(',')
                  > double_ > -lit(',')
                  > double_)[matrix_action]
    > lit(')');

auto const translate = x3::rule<class translate_tag> {} = no_case[lit("translate")]
    > lit('(') > (double_ > -lit(',') >> -double_)[translate_action] > lit(')');

auto const scale = x3::rule<class scale_tag> {} = no_case[lit("scale")]
    > lit('(') >  (double_ >  -lit(',') >  -double_)[scale_action] > lit(')');

auto const rotate = x3::rule<class rotate_tag> {} = no_case[lit("rotate")]
    > lit('(')
    > (double_ > -lit(',') > -double_ > -lit(',') > -double_)[rotate_action]
    > lit(')') ;

auto const skewX = x3::rule<class skewX_tag> {} = no_case[lit("skewX")]
    > lit('(') > double_ [ skewX_action] > lit(')');
auto const skewY = x3::rule<class skewY_tag> {} = no_case[lit("skewY")]
    > lit('(') > double_ [ skewY_action] > lit(')');

auto const transform = x3::rule<class transform_tag> {} = matrix | rotate | translate | scale /*| rotate*/ | skewX | skewY ;

auto const svg_transform_def = +transform ;

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
BOOST_SPIRIT_DEFINE(
    svg_transform
    );
#pragma GCC diagnostic pop

}

grammar::svg_transform_grammar_type const& svg_transform_grammar()
{
    return grammar::svg_transform;
}

}}


#endif // MAPNIK_SVG_TRANSFORM_GRAMMAR_X3_HPP
