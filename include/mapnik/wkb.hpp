/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id: wkb.hpp 39 2005-04-10 20:39:53Z pavlenko $

#ifndef WKB_HPP
#define WKB_HPP

#include <mapnik/geometry.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/feature.hpp>

namespace mapnik
{
    enum wkbFormat
    {
        wkbGeneric=1,
        wkbSpatiaLite=2
    };

    class MAPNIK_DECL geometry_utils 
    {
    public:

       static void from_wkb (Feature & feature,
                             const char* wkb,
                             unsigned size,
                             bool multiple_geometries = false,
                             wkbFormat format = wkbGeneric);
    private:
       geometry_utils();
       geometry_utils(geometry_utils const&);
       geometry_utils& operator=(const geometry_utils&);
    };
}

#endif //WKB_HPP
