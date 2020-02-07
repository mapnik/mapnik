/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2020 Artem Pavlenko
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
    const double DPI = 90;
    css_unit_value()
    {
        add
            ("px", 1.0)
            ("pt", DPI/72.0)
            ("pc", DPI/6.0)
            ("mm", DPI/25.4)
            ("cm", DPI/2.54)
            ("in", static_cast<double>(DPI))
            //("em", 1.0/16.0) // default pixel size for body (usually 16px)
            // ^^ this doesn't work currently as 'e' in 'em' interpreted as part of scientific notation.
            ;
    }
};

} //mapnik

#endif //MAPNIK_CSS_GRAMMAR_X3_DEF_HPP
