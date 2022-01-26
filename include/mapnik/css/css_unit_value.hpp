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

#ifndef MAPNIK_CSS_UNIT_VALUE_HPP
#define MAPNIK_CSS_UNIT_VALUE_HPP

#include <boost/spirit/home/x3.hpp>

namespace mapnik {

namespace x3 = boost::spirit::x3;

// units
struct css_unit_value : x3::symbols<double>
{
    constexpr static double DPI = 96;
    css_unit_value()
    {
        add                                //
          ("px", 1.0)                      // pixels
          ("pt", DPI / 72.0)               // points
          ("pc", DPI / 6.0)                // picas
          ("mm", DPI / 25.4)               // milimeters
          ("Q", DPI / 101.6)               // quarter-milimeters
          ("cm", DPI / 2.54)               // centimeters
          ("in", static_cast<double>(DPI)) // inches
          ;
    }
};

struct css_absolute_size : x3::symbols<double>
{
    constexpr static double EM = 10.0; // 1em == 10px
    css_absolute_size()
    {
        add                      //
          ("xx-small", 0.6 * EM) //
          ("x-small", 0.75 * EM) //
          ("small", 0.88 * EM)   //
          ("medium", 1.0 * EM)   //
          ("large", 1.2 * EM)    //
          ("x-large", 1.5 * EM)  //
          ("xx-large", 2.0 * EM) //
          ;
    }
};

struct css_relative_size : x3::symbols<double>
{
    css_relative_size()
    {
        add               //
          ("larger", 1.2) //
          ("smaller", 0.8);
    }
};

} // namespace mapnik

#endif // MAPNIK_CSS_GRAMMAR_X3_DEF_HPP
