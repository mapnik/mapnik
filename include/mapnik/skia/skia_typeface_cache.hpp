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


#if defined(HAVE_SKIA)

#ifndef MAPNIK_SKIA_TYPEFACE_CACHE_HPP
#define MAPNIK_SKIA_TYPEFACE_CACHE_HPP

#include <iostream>

class SkTypeface;

namespace mapnik
{

class skia_typeface_cache
{
public:
    skia_typeface_cache();
    ~skia_typeface_cache();
    static bool register_font(std::string const& file_name);
    static bool register_fonts(std::string const& dir, bool recurse = false);

private:
#ifdef MAPNIK_THREADSAFE
    static boost::mutex mutex_;
#endif
    static std::map<std::string, SkTypeface*> typefaces_;
};

}

#endif // MAPNIK_SKIA_TYPEFACE_CACHE_HPP
#endif // #if defined(HAVE_SKIA)
