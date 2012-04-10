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
#include <mapnik/text_placements/base.hpp>

namespace mapnik {
text_placements::text_placements() : defaults()
{
}

void text_placements::add_expressions(expression_set &output)
{
    defaults.add_expressions(output);
}


/************************************************************************/

text_placement_info::text_placement_info(text_placements const* parent,
                                         double scale_factor_)
    : properties(parent->defaults),
      scale_factor(scale_factor_)
{

}

} //ns mapnik
