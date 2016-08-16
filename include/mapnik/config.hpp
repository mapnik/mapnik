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

#ifndef MAPNIK_CONFIG_HPP
#define MAPNIK_CONFIG_HPP

// Windows DLL support

#ifdef _WINDOWS
#  define MAPNIK_EXP __declspec (dllexport)
#  define MAPNIK_IMP __declspec (dllimport)
#  ifdef MAPNIK_EXPORTS
#    define MAPNIK_DECL __declspec (dllexport)
#  else
#    define MAPNIK_DECL __declspec (dllimport)
#  endif
#  pragma warning( disable: 4251 )
#  pragma warning( disable: 4275 )
#  if (_MSC_VER >= 1400) // vc8
#    pragma warning(disable : 4996) //_CRT_SECURE_NO_DEPRECATE
#  endif
#else
#  if __GNUC__ >= 4
#  define MAPNIK_EXP __attribute__ ((visibility ("default")))
#  define MAPNIK_DECL __attribute__ ((visibility ("default")))
#  define MAPNIK_IMP __attribute__ ((visibility ("default")))
#  else
#  define MAPNIK_EXP
#  define MAPNIK_DECL
#  define MAPNIK_IMP
#  endif
#endif

#define PROJ_ENVELOPE_POINTS 20

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS

#ifndef BOOST_MPL_LIMIT_VECTOR_SIZE
 #define BOOST_MPL_LIMIT_VECTOR_SIZE 30
#else
 #warning "WARNING: BOOST_MPL_LIMIT_VECTOR_SIZE is already defined. Ensure config.hpp is included before any Boost headers"
#endif

#ifndef BOOST_MPL_LIMIT_LIST_SIZE
 #define BOOST_MPL_LIMIT_LIST_SIZE 30
#else
 #warning "WARNING: BOOST_MPL_LIMIT_LIST_SIZE is already defined. Ensure config.hpp is included before any Boost headers"
#endif

#endif // MAPNIK_CONFIG_HPP
