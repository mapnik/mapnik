/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
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
#include <mapnik/image_filter_types.hpp>
#include <mapnik/image_filter_grammar.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/spirit/include/phoenix.hpp>
#pragma GCC diagnostic pop

namespace { // internal

    BOOST_PHOENIX_ADAPT_FUNCTION(
        typename std::remove_reference<A1>::type, ovo, // = optional_value_or
        boost::get_optional_value_or, 2)

} // namespace internal

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
    qi::_2_type _2;
    qi::_3_type _3;
    qi::_4_type _4;
    qi::_5_type _5;
    qi::_6_type _6;
    qi::_7_type _7;
    qi::_8_type _8;
    qi::_a_type _a;
    qi::attr_type attr;
    qi::double_type double_;
    qi::hold_type hold;
    qi::omit_type omit;
    using phoenix::push_back;
    using phoenix::construct;

    start = -(filter % *lit(','))
        ;

    filter = omit[alternatives[_a = _1]] >> qi::lazy(*_a)
        ;

    add("emboss") = no_args >> attr(construct<mapnik::filter::emboss>());
    add("blur") = no_args >> attr(construct<mapnik::filter::blur>());
    add("gray") = no_args >> attr(construct<mapnik::filter::gray>());
    add("edge-detect") = no_args >> attr(construct<mapnik::filter::edge_detect>());
    add("sobel") = no_args >> attr(construct<mapnik::filter::sobel>());
    add("sharpen") = no_args >> attr(construct<mapnik::filter::sharpen>());
    add("x-gradient") = no_args >> attr(construct<mapnik::filter::x_gradient>());
    add("y-gradient") = no_args >> attr(construct<mapnik::filter::y_gradient>());
    add("invert") = no_args >> attr(construct<mapnik::filter::invert>());
    add("color-blind-protanope") = no_args >> attr(construct<mapnik::filter::color_blind_protanope>());
    add("color-blind-deuteranope") = no_args >> attr(construct<mapnik::filter::color_blind_deuteranope>());
    add("color-blind-tritanope") = no_args >> attr(construct<mapnik::filter::color_blind_tritanope>());

    add("agg-stack-blur") =
        (lit('(') >> radius_ >> -( lit(',') >> radius_ ) >> lit(')'))
        [push_back(_val, construct<filter::agg_stack_blur>(_1, ovo(_2, _1)))]
        |
        no_args
        [push_back(_val, construct<filter::agg_stack_blur>(1, 1))]
        ;

    add("scale-hsla") =
        (lit('(')
          >> double_ >> lit(',') >> double_ >> lit(',')
          >> double_ >> lit(',') >> double_ >> lit(',')
          >> double_ >> lit(',') >> double_ >> lit(',')
          >> double_ >> lit(',') >> double_ >> lit(')'))
        [push_back(_val, construct<filter::scale_hsla>(_1,_2,_3,_4,_5,_6,_7,_8))]
        ;

    add("colorize-alpha") = qi::as<filter::colorize_alpha>()
        [lit('(') >> color_stop_ % lit(',') >> lit(')')]
        [push_back(_val, _1)]
        ;

    color_stop_ = (css_color_ >> -color_stop_offset)
        [_val = construct<filter::color_stop>(_1, ovo(_2, 0.0))]
        ;

    color_stop_offset = double_[_val = _1]
        >> -lit('%')[_val = percent_offset(_val)]
        ;

    add("color-to-alpha") =
        hold[lit('(') >> css_color_ >> lit(')')]
        [push_back(_val, construct<filter::color_to_alpha>(_1))]
        ;

    no_args = -(lit('(') >> lit(')'));
}

template <typename Iterator, typename ContType>
auto image_filter_grammar<Iterator, ContType>::add(std::string const& symbol)
    -> alternative_type &
{
    if (num_alternatives >= max_alternatives)
    {
        throw std::length_error("too many alternatives in image_filter_grammar");
    }

    alternative_storage[num_alternatives].name(symbol);
    alternatives.add(symbol, &alternative_storage[num_alternatives]);
    return alternative_storage[num_alternatives++];
}

} // namespace mapnik
