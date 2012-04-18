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
#include <mapnik/group_symbolizer.hpp>

// stl
#include <iostream>

namespace mapnik
{

group_symbolizer::group_symbolizer(text_placements_ptr placements)
   : placements_(placements)
{
}

text_placements_ptr group_symbolizer::get_placement_options() const
{
   return placements_;
}

void group_symbolizer::set_placement_options(text_placements_ptr placement_options)
{
   placements_ = placement_options;
}

void group_symbolizer::add_rule(const group_rule &r)
{
   group_rules_.push_back(r);
}

}

