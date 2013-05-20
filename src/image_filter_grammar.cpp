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

// boost
#include <boost/version.hpp>

// spirit
#include <boost/spirit/include/phoenix.hpp>

namespace mapnik {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

template <typename Iterator, typename ContType>
image_filter_grammar<Iterator,ContType>::image_filter_grammar()
    : image_filter_grammar::base_type(start)
{
    using qi::lit;
    using qi::_val;
    using qi::_1;
    using qi::_a;
    using qi::_b;
    using qi::_c;
    using qi::_d;
    using qi::_e;
    using qi::_f;
    using qi::_g;
    using qi::_h;
    using qi::_r1;
    using qi::eps;
    using qi::char_;
    using qi::lexeme;
    using qi::double_;
    using boost::spirit::ascii::string;
    using phoenix::push_back;
    using phoenix::construct;
    using phoenix::at_c;
#if BOOST_VERSION >= 104700
    using qi::no_skip;
    start = -(filter % no_skip[*char_(", ")])
        ;
#else
    start = -(filter)
        ;
#endif

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
        //hsla_filter(_val)
        //|
        colorize_alpha_filter(_val)
        ;

    agg_blur_filter = lit("agg-stack-blur")[_a = 1, _b = 1]
        >> -( lit('(') >> -( radius_[_a = _1][_b = _1]
                             >> -(lit(',') >> radius_[_b = _1]))
              >> lit(')'))
        [push_back(_r1,construct<mapnik::filter::agg_stack_blur>(_a,_b))]
        ;

    /*
    hsla_filter = lit("hsla")
        >> lit('(')
        >> double_[_a = _1] >> lit('x') >> double_[_b = _1] >> lit(';')
        >> double_[_c = _1] >> lit('x') >> double_[_d = _1] >> lit(';')
        >> double_[_e = _1] >> lit('x') >> double_[_f = _1] >> lit(';')
        >> double_[_g = _1] >> lit('x') >> double_[_h = _1] >> lit(')')
        [push_back(_r1, construct<mapnik::filter::hsla>(_a,_b,_c,_d,_e,_f,_g,_h))]
        ;
    */

    colorize_alpha_filter = lit("colorize-alpha")[_a = construct<mapnik::filter::colorize_alpha>()]
        >> lit('(')
        >> (css_color_[at_c<0>(_b) = _1, at_c<1>(_b) = 0]
            >> -color_stop_offset(_b)) [push_back(_a,_b)]
        >> +(lit(',') >> css_color_[at_c<0>(_b) =_1,at_c<1>(_b) = 0]
             >> -color_stop_offset(_b))[push_back(_a,_b)]
        >> lit(')') [push_back(_r1,_a)]
        ;

    color_stop_offset = (double_ >> lit('%'))[at_c<1>(_r1) = percent_offset(_1)]
        |
        double_[at_c<1>(_r1) = _1]
        ;
    no_args = -(lit('(') >> lit(')'));
}

template struct mapnik::image_filter_grammar<std::string::const_iterator,std::vector<mapnik::filter::filter_type> >;

}
