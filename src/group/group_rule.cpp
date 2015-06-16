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
#include <mapnik/symbolizer.hpp>
#include <mapnik/group/group_rule.hpp>

// stl
#include <iostream>

namespace mapnik
{

group_rule::group_rule(const expression_ptr& filter,
                       const expression_ptr& repeat_key)
   : filter_(filter),
     repeat_key_(repeat_key)
{
}

group_rule &group_rule::operator=(const group_rule &rhs)
{
   group_rule tmp(rhs);
   filter_.swap(tmp.filter_);
   symbolizers_.swap(tmp.symbolizers_);
   return *this;
}

bool group_rule::operator==(const group_rule &rhs) const
{
   return (this == &rhs);
}

void group_rule::append(const mapnik::symbolizer &sym)
{
   symbolizers_.push_back(sym);
}

}
