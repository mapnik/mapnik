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

#ifndef MAPNIK_GEOMETRY_TO_DS_TYPE
#define MAPNIK_GEOMETRY_TO_DS_TYPE

// mapnik
#include <mapnik/global.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/datasource.hpp>

// boost
#include <boost/optional.hpp>

namespace mapnik { namespace util {

    static inline void to_ds_type(mapnik::geometry_container const& paths,
                    boost::optional<mapnik::datasource::geometry_t> & result)
    {
        if (paths.size() == 1)
        {
            result.reset(static_cast<mapnik::datasource::geometry_t>(paths.front().type()));
        }
        else if (paths.size() > 1)
        {
            int multi_type = 0;
            for (auto const& geom : paths)
            {
                int type = static_cast<int>(geom.type());
                if (multi_type > 0 && multi_type != type)
                {
                    result.reset(datasource::Collection);
                }
                multi_type = type;
                result.reset(static_cast<mapnik::datasource::geometry_t>(type));
            }
        }
    }

    }}


#endif // MAPNIK_GEOMETRY_TO_DS_TYPE
