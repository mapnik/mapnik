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

#ifndef MAPNIK_GROUP_SYMBOLIZER_HPP
#define MAPNIK_GROUP_SYMBOLIZER_HPP

// mapnik
#include <mapnik/symbolizer.hpp>
#include <mapnik/text_symbolizer.hpp>
#include <mapnik/group_rule.hpp>
#include <mapnik/group_layout.hpp>

// boost
#include <boost/shared_ptr.hpp>

namespace mapnik
{

/**
 * group symbolizer places a group of other symbolizers
 * at points or along lines. rules within the group mean
 * that matching sets of symbolizers will be rendered
 * atomically - all or none.
 */
struct MAPNIK_DECL group_symbolizer : public symbolizer_base
{
   typedef std::vector<mapnik::group_rule> rules;

   explicit group_symbolizer(size_t column_index_start, 
                             size_t column_index_end,
                             const group_layout &layout = simple_row_layout(),
                             text_placements_ptr placements = text_placements_ptr(new text_placements_dummy));

   text_placements_ptr get_placement_options() const;
   void set_placement_options(text_placements_ptr placement_options);

   void add_rule(const group_rule &);

   const rules &get_rules() const
   {
      return group_rules_;
   }

   inline rules::const_iterator begin() const 
   {
      return group_rules_.begin();
   }

   inline rules::const_iterator end() const
   {
      return group_rules_.end();
   }

   inline size_t get_column_index_start() const
   {
      return column_index_start_;
   }

   inline size_t get_column_index_end() const
   {
      return column_index_end_;
   }

   inline const group_layout &get_layout() const
   {
      return layout_;
   }

   inline void set_layout(const group_layout &layout)
   {
      layout_ = layout;
   }

private:

   // start and end column indexes
   size_t column_index_start_, column_index_end_;

   // placement parameters?
   text_placements_ptr placements_;

   // object to represent the type of layout for the group, and it's parameters
   group_layout layout_;

   // the rules which are evaluated as part of the symbolizer
   rules group_rules_;
};
}

#endif // MAPNIK_GROUP_SYMBOLIZER_HPP
