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

#ifndef MAPNIK_FONT_LIBRARY_HPP
#define MAPNIK_FONT_LIBRARY_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/util/noncopyable.hpp>

// stl
#include <memory>

struct FT_LibraryRec_;
struct FT_MemoryRec_;

namespace mapnik {

class MAPNIK_DECL font_library : public util::noncopyable
{
  public:
    explicit font_library();
    ~font_library();
    FT_LibraryRec_* get();

  private:
    FT_LibraryRec_* library_;
    std::unique_ptr<FT_MemoryRec_> memory_;
};

} // namespace mapnik

#endif // MAPNIK_FONT_LIBRARY_HPP
