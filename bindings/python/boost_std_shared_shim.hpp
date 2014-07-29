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

#ifndef MAPNIK_PYTHON_BOOST_STD_SHARED_SHIM
#define MAPNIK_PYTHON_BOOST_STD_SHARED_SHIM

// boost
#include <boost/version.hpp>
#include <boost/config.hpp>

#if BOOST_VERSION < 105300 || defined BOOST_NO_CXX11_SMART_PTR

// https://github.com/mapnik/mapnik/issues/2022
#include <memory>

namespace boost {
template<class T> const T* get_pointer(std::shared_ptr<T> const& p)
{
    return p.get();
}

template<class T> T* get_pointer(std::shared_ptr<T>& p)
{
    return p.get();
}
} // namespace boost

#endif

#endif // MAPNIK_PYTHON_BOOST_STD_SHARED_SHIM
