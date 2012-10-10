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

#ifndef MAPNIK_LINE_PATTERN_SYMBOLIZER_HPP
#define MAPNIK_LINE_PATTERN_SYMBOLIZER_HPP

// mapnik
#include <mapnik/symbolizer.hpp>
//#include <boost/shared_ptr.hpp>

namespace mapnik
{
struct MAPNIK_DECL line_pattern_symbolizer :
        public symbolizer_with_image, public symbolizer_base
{
    line_pattern_symbolizer(path_expression_ptr file);
    line_pattern_symbolizer(line_pattern_symbolizer const& rhs);
};
}

#endif // MAPNIK_LINE_PATTERN_SYMBOLIZER_HPP
