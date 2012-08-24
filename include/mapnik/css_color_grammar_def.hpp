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

#ifndef MAPNIK_CSS_COLOR_GRAMMAR_DEF_HPP
#define MAPNIK_CSS_COLOR_GRAMMAR_DEF_HPP

// boost
#include <boost/version.hpp>

#if BOOST_VERSION >= 104500

#include <mapnik/css_color_grammar.hpp>

namespace mapnik
{

template <typename Iterator>
css_color_grammar<Iterator>::css_color_grammar()
    : css_color_grammar::base_type(css_color)

{
    using qi::lit;
    using qi::_val;
    using qi::double_;
    using qi::_1;
    using qi::_a;
    using qi::_b;
    using qi::_c;
    using ascii::no_case;
    using phoenix::at_c;

    css_color %= rgba_color
        | rgba_percent_color
        | hsl_percent_color
        | hex_color
        | hex_color_small
        | no_case[named];

    hex_color = lit('#')
        >> hex2 [ at_c<0>(_val) = _1 ]
        >> hex2 [ at_c<1>(_val) = _1 ]
        >> hex2 [ at_c<2>(_val) = _1 ]
        >>-hex2 [ at_c<3>(_val) = _1 ]
        ;

    hex_color_small = lit('#')
        >> hex1 [ at_c<0>(_val) = _1 | _1 << 4 ]
        >> hex1 [ at_c<1>(_val) = _1 | _1 << 4 ]
        >> hex1 [ at_c<2>(_val) = _1 | _1 << 4 ]
        >>-hex1 [ at_c<3>(_val) = _1 | _1 << 4 ]
        ;

    rgba_color = lit("rgb") >> -lit('a')
                            >> lit('(')
                            >> dec3 [at_c<0>(_val) = _1] >> ','
                            >> dec3 [at_c<1>(_val) = _1] >> ','
                            >> dec3 [at_c<2>(_val) = _1]
                            >> -(','>> -double_ [at_c<3>(_val) = alpha_converter(_1)])
                            >> lit(')')
        ;

    rgba_percent_color = lit("rgb") >> -lit('a')
                                    >> lit('(')
                                    >> double_ [at_c<0>(_val) = percent_converter(_1)] >> '%' >> ','
                                    >> double_ [at_c<1>(_val) = percent_converter(_1)] >> '%' >> ','
                                    >> double_ [at_c<2>(_val) = percent_converter(_1)] >> '%'
                                    >> -(','>> -double_ [at_c<3>(_val) = alpha_converter(_1)])
                                    >> lit(')')
        ;

    hsl_percent_color = lit("hsl") >> -lit('a')
                                   >> lit('(')
                                   >> double_ [ _a = _1] >> ','        // hue 0..360
                                   >> double_ [ _b = _1] >> '%' >> ',' // saturation 0..100%
                                   >> double_ [ _c = _1] >> '%'        // lightness  0..100%
                                   >> -(','>> -double_ [at_c<3>(_val) = alpha_converter(_1)]) // opacity 0...1
                                   >> lit (')') [ hsl_converter(_val,_a,_b,_c)]
        ;
}

}

#endif

#endif