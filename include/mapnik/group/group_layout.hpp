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

#ifndef MAPNIK_GROUP_LAYOUT_HPP
#define MAPNIK_GROUP_LAYOUT_HPP

// mapnik
#include <mapnik/util/variant.hpp>
// std
#include <memory>

namespace mapnik
{

struct simple_row_layout
{
public:
   simple_row_layout(double item_margin = 0.0)
      : item_margin_(item_margin)
   {
   }

   double get_item_margin() const
   {
      return item_margin_;
   }

   void set_item_margin(double item_margin)
   {
      item_margin_ = item_margin;
   }

private:
   double item_margin_;
};

struct pair_layout
{
public:
   pair_layout(double item_margin = 1.0, double max_difference = - 1.0)
      : item_margin_(item_margin),
        max_difference_(max_difference)
   {
   }

   double get_item_margin() const
   {
      return item_margin_;
   }

   void set_item_margin(double item_margin)
   {
      item_margin_ = item_margin;
   }

   double get_max_difference() const
   {
      return max_difference_;
   }

   void set_max_difference(double max_difference)
   {
      max_difference_ = max_difference;
   }

private:
   double item_margin_;
   double max_difference_;
};

using group_layout = util::variant<simple_row_layout,pair_layout>;
using group_layout_ptr = std::shared_ptr<group_layout>;
}

#endif // MAPNIK_GROUP_LAYOUT_HPP
