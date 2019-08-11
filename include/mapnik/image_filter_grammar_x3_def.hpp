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

#ifndef MAPNIK_IMAGE_FILTER_GRAMMAR_X3_DEF_HPP
#define MAPNIK_IMAGE_FILTER_GRAMMAR_X3_DEF_HPP


#include <mapnik/image_filter_grammar_x3.hpp>
#include <mapnik/image_filter_types.hpp>
#include <mapnik/css_color_grammar_x3.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/adapted/std_tuple.hpp> // spirit support
#pragma GCC diagnostic pop


BOOST_FUSION_ADAPT_STRUCT(
    mapnik::filter::scale_hsla,
    (double, h0)
    (double, h1)
    (double, s0)
    (double, s1)
    (double, l0)
    (double, l1)
    (double, a0)
    (double, a1)
)

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::filter::color_stop,
    (mapnik::color, color )
    (double, offset)
)

BOOST_FUSION_ADAPT_STRUCT(
    mapnik::filter::color_to_alpha,
    (mapnik::color, color)
)

namespace mapnik {

namespace x3 = boost::spirit::x3;

namespace image_filter {

using x3::lit;
using x3::uint_parser;
using x3::hex;
using x3::symbols;
using x3::omit;
using x3::attr;
using x3::double_;
using x3::no_case;
using x3::no_skip;
using x3::char_;

auto push_back = [](auto& ctx)
{
    _val(ctx).push_back(_attr(ctx));

};

auto set_rx_ry = [](auto & ctx)
{
    _val(ctx).rx = _val(ctx).ry = _attr(ctx);
};

auto set_ry = [](auto & ctx)
{
    _val(ctx).ry = _attr(ctx);
};

auto offset_value = [](auto & ctx)
{
    _val(ctx) = _attr(ctx);
};

auto percent = [](auto & ctx)
{
    double val = std::abs(_val(ctx)/100.0);
    if (val > 1.0) val = 1.0;
    _val(ctx) = val;
};

x3::uint_parser<unsigned, 10, 1, 3> radius;

// Import the expression rule
namespace { using css_color_grammar::css_color; }

// rules
x3::rule<class filter_class, filter::filter_type > const filter("filter");

x3::rule<class emboss_class, filter::emboss> const emboss_filter("emboss");
x3::rule<class blur_class, filter::blur> const blur_filter("blur");
x3::rule<class gray_class, filter::gray> const gray_filter("gray");
x3::rule<class edge_detect_class, filter::edge_detect> const edge_detect_filter("edge-detect");
x3::rule<class sobel_class, filter::sobel> const sobel_filter("sobel");
x3::rule<class sharpen_class, filter::sharpen> const sharpen_filter("sharpen");
x3::rule<class x_gradient_class, filter::x_gradient> const x_gradient_filter("x-gradient");
x3::rule<class y_gradient_class, filter::y_gradient> const y_gradient_filter("y-gradient");
x3::rule<class invert_class, filter::invert> const invert_filter("invert");
x3::rule<class color_blind_protanope_class, filter::color_blind_protanope> const color_blind_protanope_filter("color-blind-protanope");
x3::rule<class color_blind_deuteranope_class, filter::color_blind_deuteranope> const color_blind_deuteranope_filter("color-blind-deuteranope");
x3::rule<class color_blind_tritanope_class, filter::color_blind_tritanope> const color_blind_tritanope_filter("color-blind-tritanope");

x3::rule<class agg_blur_class, filter::agg_stack_blur> const agg_blur_filter("agg blur filter");
x3::rule<class scale_hsla_class, filter::scale_hsla> const scale_hsla_filter("scale-hsla");
x3::rule<class colorize_alpha_class, filter::colorize_alpha> const colorize_alpha_filter("colorize-alpha");
x3::rule<class color_stop_class, filter::color_stop> const color_stop("color-stop");
x3::rule<class offset_class, double> const offset("color-stop-offset");
x3::rule<class color_to_alpha_class, filter::color_to_alpha> const color_to_alpha_filter("color-to-alpha");

auto const no_args = -(lit('(') > lit(')'));

auto const start_def = -(filter[push_back] % *lit(','));

auto const filter_def = (emboss_filter
                         |
                         blur_filter
                         |
                         gray_filter
                         |
                         edge_detect_filter
                         |
                         sobel_filter
                         |
                         sharpen_filter
                         |
                         x_gradient_filter
                         |
                         y_gradient_filter
                         |
                         invert_filter
                         |
                         color_blind_protanope_filter
                         |
                         color_blind_deuteranope_filter
                         |
                         color_blind_tritanope_filter
                         |
                         agg_blur_filter
                         |
                         scale_hsla_filter
                         |
                         colorize_alpha_filter
                         |
                         color_to_alpha_filter
                         )
    ;

auto const emboss_filter_def = lit("emboss") > no_args;

auto const blur_filter_def = lit("blur") > no_args;

auto const gray_filter_def = lit("gray") > no_args;

auto const edge_detect_filter_def = lit("edge-detect") > no_args;

auto const sobel_filter_def = lit("sobel") > no_args;

auto const sharpen_filter_def = lit("sharpen") > no_args;

auto const x_gradient_filter_def = lit("x-gradient") > no_args;

auto const y_gradient_filter_def = lit("y-gradient") > no_args;

auto const invert_filter_def = lit("invert") > no_args;

auto const color_blind_protanope_filter_def = lit("color-blind-protanope") > no_args;

auto const color_blind_deuteranope_filter_def = lit("color-blind-deuteranope") > no_args;

auto const color_blind_tritanope_filter_def = lit("color-blind-tritanope") > no_args;

auto const agg_blur_filter_def = lit("agg-stack-blur")
    > -(lit('(') > -(radius[set_rx_ry] > -(lit(',') > radius[set_ry])) > lit(')'));

auto const scale_hsla_filter_def = lit("scale-hsla") > lit('(')
    > double_ > ','
    > double_ > ','
    > double_ > ','
    > double_ > ','
    > double_ > ','
    > double_ > ','
    > double_ > ','
    > double_ > ')' ;


auto const offset_def = double_[offset_value] > -lit('%')[percent];
auto const color_stop_def = css_color > -offset;

auto const colorize_alpha_filter_def = lit("colorize-alpha")
    > lit('(')
    > color_stop > *(lit(',') > color_stop)
    > lit(')') ;

auto const color_to_alpha_filter_def = lit("color-to-alpha") > lit('(')
    > -css_color > lit(')');

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>

BOOST_SPIRIT_DEFINE(
    start,
    filter,
    emboss_filter,
    blur_filter,
    gray_filter,
    edge_detect_filter,
    sobel_filter,
    sharpen_filter,
    x_gradient_filter,
    y_gradient_filter,
    invert_filter,
    agg_blur_filter,
    color_blind_protanope_filter,
    color_blind_deuteranope_filter,
    color_blind_tritanope_filter,
    scale_hsla_filter,
    colorize_alpha_filter,
    color_stop,
    offset,
    color_to_alpha_filter
    );
#pragma GCC diagnostic pop

} // image_filter
} //ns mapnik

#endif //MAPNIK_IMAGE_FILTER_GRAMMAR_X3_DEF_HPP
