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

#ifndef MAPNIK_SKIA_FONT_MANAGER_HPP
#define MAPNIK_SKIA_FONT_MANAGER_HPP

#include <iostream>
#include <mapnik/unicode.hpp>

namespace mapnik
{

class skia_typeface_cache;

class skia_font_manager
{
public:

    skia_font_manager(skia_typeface_cache & cache)
        : cache_(cache) {}

    void test(std::string const&, UnicodeString & ustr);
private:
    skia_typeface_cache & cache_;
};

}

#endif // MAPNIK_SKIA_FONT_MANAGER_HPP
#endif // #if defined(HAVE_SKIA)
