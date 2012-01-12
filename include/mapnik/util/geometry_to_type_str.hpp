/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2012 Artem Pavlenko
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

//$Id$

#ifndef MAPNIK_GEOMETRY_DESCRIBE
#define MAPNIK_GEOMETRY_DESCRIBE

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/geometry.hpp>


namespace mapnik { namespace util {

void to_type_str(mapnik::geometry_type const& g, std::string & geometry_string_type )
{
    switch (g.type())
    {
    case mapnik::Point:
        geometry_string_type = "point";
        break;
    case mapnik::LineString:
        geometry_string_type = "linestring";
        break;
    case mapnik::Polygon:
        geometry_string_type = "polygon";
        break;
    default:
        break;
    }
}

void to_type_str(mapnik::geometry_container const& paths, std::string & geometry_string_type )
{
    if (paths.size() == 1)
    {
        // single geometry
        to_type_str(paths.front(), geometry_string_type);
    }
    else if (paths.size() > 1)
    {
        int multi_type = 0;
        geometry_container::const_iterator itr = paths.begin();
        geometry_container::const_iterator end = paths.end();
        for ( ; itr!=end; ++itr)
        {
            int type = static_cast<int>(itr->type());
            if (multi_type > 0 && multi_type != itr->type())
            {
                geometry_string_type = "collection";
                break;
            }
            to_type_str(*itr, geometry_string_type);
            multi_type = type;
        }
    }
}
                                     
}}


#endif // MAPNIK_GEOMETRY_DESCRIBE
