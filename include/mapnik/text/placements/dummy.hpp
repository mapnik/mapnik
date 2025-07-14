/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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
#ifndef PLACEMENTS_DUMMY_HPP
#define PLACEMENTS_DUMMY_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/text/placements/base.hpp>

namespace mapnik {

class text_placement_info_dummy;
class feature_impl;
struct attribute;

// Dummy placement algorithm. Always takes the default value.
class MAPNIK_DECL text_placements_dummy : public text_placements
{
  public:
    text_placement_info_ptr
      get_placement_info(double _scale_factor, feature_impl const& feature, attributes const& vars) const;
    friend class text_placement_info_dummy;
};

// Placement info object for dummy placement algorithm. Always takes the default value.
class MAPNIK_DECL text_placement_info_dummy : public text_placement_info
{
  public:
    text_placement_info_dummy(text_placements_dummy const* parent, double _scale_factor)
        : text_placement_info(parent, _scale_factor),
          state(0)
    {}
    bool next() const;

  private:
    mutable unsigned state;
};

} // namespace mapnik

#endif // PLACEMENTS_DUMMY_HPP
