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

#ifndef MAPNIK_MAKE_UNIQUE_HPP
#define MAPNIK_MAKE_UNIQUE_HPP

#include <memory>

// http://stackoverflow.com/questions/14131454/visual-studio-2012-cplusplus-and-c-11
#if defined(_MSC_VER) && _MSC_VER < 1800 || !defined(_MSC_VER) && __cplusplus <= 201103L

namespace std {

// C++14 backfill from http://herbsutter.com/gotw/_102/
template<typename T, typename ...Args>
inline std::unique_ptr<T> make_unique(Args&& ...args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}
#endif

#endif // MAPNIK_MAKE_UNIQUE_HPP
