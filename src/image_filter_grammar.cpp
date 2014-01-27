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

// mapnik
#include <mapnik/image_filter_grammar.hpp>

// spirit
#include <boost/spirit/include/phoenix.hpp>

namespace mapnik {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

template <typename Iterator, typename ContType>
image_filter_grammar<Iterator,ContType>::image_filter_grammar()
    : image_filter_grammar::base_type(start)
{
    qi::lit_type lit;
    qi::_val_type _val;
    qi::_1_type _1;
    qi::_a_type _a;
    qi::_b_type _b;
    qi::_c_type _c;
    qi::_d_type _d;
    qi::_e_type _e;
    qi::_f_type _f;
    qi::_g_type _g;
    qi::_h_type _h;
    qi::_r1_type _r1;
    qi::eps_type eps;
    qi::char_type char_;
    qi::double_type double_;
    qi::no_skip_type no_skip;
    using phoenix::push_back;
    using phoenix::construct;
    using phoenix::at_c;
    start = -(filter % no_skip[*char_(", ")])
        ;

    filter =
        lit("emboss") >> no_args [push_back(_val,construct<mapnik::filter::emboss>())]
        |
        lit("blur") >> no_args [push_back(_val,construct<mapnik::filter::blur>())]
        |
        lit("gray") >> no_args [push_back(_val,construct<mapnik::filter::gray>())]
        |
        lit("edge-detect") >> no_args [push_back(_val,construct<mapnik::filter::edge_detect>())]
        |
        lit("sobel") >> no_args [push_back(_val,construct<mapnik::filter::sobel>())]
        |
        lit("sharpen") >> no_args [push_back(_val,construct<mapnik::filter::sharpen>())]
        |
        lit("x-gradient") >> no_args [push_back(_val,construct<mapnik::filter::x_gradient>())]
        |
        lit("y-gradient") >> no_args [push_back(_val,construct<mapnik::filter::y_gradient>())]
        |
        lit("invert") >> no_args [push_back(_val,construct<mapnik::filter::invert>())]
        |
        agg_blur_filter(_val)
        |
        scale_hsla_filter(_val)
        |
        colorize_alpha_filter(_val)
        |
        color_to_alpha_filter(_val)
        ;

    agg_blur_filter = lit("agg-stack-blur")[_a = 1, _b = 1]
        >> -( lit('(') >> -( radius_[_a = _1][_b = _1]
                             >> -(lit(',') >> radius_[_b = _1]))
              >> lit(')'))
        [push_back(_r1,construct<mapnik::filter::agg_stack_blur>(_a,_b))]
        ;

    scale_hsla_filter = lit("scale-hsla")
        >> lit('(')
        >> double_[_a = _1] >> lit(',') >> double_[_b = _1] >> lit(',')
        >> double_[_c = _1] >> lit(',') >> double_[_d = _1] >> lit(',')
        >> double_[_e = _1] >> lit(',') >> double_[_f = _1] >> lit(',')
        >> double_[_g = _1] >> lit(',') >> double_[_h = _1] >> lit(')')
        [push_back(_r1, construct<mapnik::filter::scale_hsla>(_a,_b,_c,_d,_e,_f,_g,_h))]
        ;

    colorize_alpha_filter = lit("colorize-alpha")[_a = construct<mapnik::filter::colorize_alpha>()]
        >> lit('(')
        >> (css_color_[at_c<0>(_b) = _1, at_c<1>(_b) = 0]
            >> -color_stop_offset(_b)) [push_back(_a,_b)]
        >> -(+(lit(',') >> css_color_[at_c<0>(_b) =_1,at_c<1>(_b) = 0]
             >> -color_stop_offset(_b))[push_back(_a,_b)])
        >> lit(')') [push_back(_r1,_a)]
        ;

    color_stop_offset = (double_ >> lit('%'))[at_c<1>(_r1) = percent_offset(_1)]
        |
        double_[at_c<1>(_r1) = _1]
        ;

    color_to_alpha_filter = lit("color-to-alpha")
        >> lit('(')
        >> css_color_[_a = _1]
        >> lit(')')
        [push_back(_r1,construct<mapnik::filter::color_to_alpha>(_a))]
        ;

    no_args = -(lit('(') >> lit(')'));
}

template struct mapnik::image_filter_grammar<std::string::const_iterator,std::vector<mapnik::filter::filter_type> >;

}
