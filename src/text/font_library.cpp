/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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
#include <mapnik/text/font_library.hpp>
#include <mapnik/safe_cast.hpp>

// stl
#include <cstdlib>
#include <stdexcept>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>

// freetype2
extern "C"
{
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_MODULE_H
}

#pragma GCC diagnostic pop

namespace {

void* _Alloc_Func(FT_Memory, long size)
{
    return std::malloc(mapnik::safe_cast<std::size_t>(size));
}

void _Free_Func(FT_Memory, void *block)
{
    std::free(block);
}

void* _Realloc_Func(FT_Memory, long /*cur_size*/, long new_size, void* block)
{
    return std::realloc(block, mapnik::safe_cast<std::size_t>(new_size));
}

}

namespace mapnik {

font_library::font_library()
  : library_(nullptr),
    memory_(new FT_MemoryRec_)
{
    memory_->alloc = _Alloc_Func;
    memory_->free = _Free_Func;
    memory_->realloc = _Realloc_Func;
    FT_Error error = FT_New_Library(&*memory_, &library_);
    if (error) throw std::runtime_error("can not initialize FreeType2 library");
    FT_Add_Default_Modules(library_);
}

FT_Library font_library::get()
{
    return library_;
}

font_library::~font_library()
{
    FT_Done_Library(library_);
}

} // end namespace mapnik
