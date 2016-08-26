/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 CartoDB
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

#ifndef POSTGIS_UTILS_HPP
#define POSTGIS_UTILS_HPP

// mapnik
#include <mapnik/geometry.hpp>
#include <mapnik/noncopyable.hpp>

namespace mapnik
{

/*!
 * TWKB is a multi-purpose format for serializing vector geometry data 
 * into a byte buffer, with an emphasis on minimizing size of the buffer.
 * https://github.com/TWKB/Specification/blob/master/twkb.md
 */

class MAPNIK_DECL postgis_utils : private mapnik::noncopyable
{
public:

    static bool from_twkb (boost::ptr_vector<geometry_type>& paths,
                          const char* twkb,
                          unsigned size);
};
}

#endif // POSTGIS_UTILS_HPP
