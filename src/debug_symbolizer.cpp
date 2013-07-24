/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2013 Artem Pavlenko
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
#include <mapnik/debug_symbolizer.hpp>
#include <mapnik/enumeration.hpp>

namespace mapnik
{

static const char * debug_symbolizer_mode_strings[] = {
    "collision",
    "vertex",
    ""
};

IMPLEMENT_ENUM( debug_symbolizer_mode_e, debug_symbolizer_mode_strings )

debug_symbolizer::debug_symbolizer()
: symbolizer_base(),
  mode_(DEBUG_SYM_MODE_COLLISION) {}

debug_symbolizer::debug_symbolizer(debug_symbolizer const& rhs)
    : symbolizer_base(rhs),
      mode_(rhs.mode_) {}

debug_symbolizer_mode_e debug_symbolizer::get_mode() const
{
    return mode_;
}

void debug_symbolizer::set_mode(debug_symbolizer_mode_e mode)
{
    mode_ = mode;
}

}

