/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2019 Artem Pavlenko
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

#ifndef MAPNIK_PLUGINS_INPUT_PGCOMMON_SQL_UTILS_HPP
#define MAPNIK_PLUGINS_INPUT_PGCOMMON_SQL_UTILS_HPP

// mapnik
#include <mapnik/geometry/box2d.hpp>

// stl
#include <algorithm>
#include <cstdio>
#include <ostream>

namespace mapnik { namespace pgcommon {


// sql_bbox

struct sql_bbox : box2d<double>
{
    int srid_;

    sql_bbox(box2d<double> const& box, int srid)
        : box2d<double>(box), srid_(srid) {}
};

inline std::ostream & operator << (std::ostream & os, sql_bbox const& box)
{
    char const fmt[] = "ST_MakeEnvelope(float8 '%a', float8 '%a', "
                                       "float8 '%a', float8 '%a', %d)";
    char buf[sizeof(fmt) + 4 * 25 + sizeof(int[3])];
    int len = std::snprintf(buf, sizeof(buf), fmt,
            box.minx(), box.miny(), box.maxx(), box.maxy(),
            std::max(box.srid_, 0));
    return os.write(buf, std::min(len, int(sizeof(buf)) - 1));
}


}} // namespace mapnik::pgcommon

#endif // MAPNIK_PLUGINS_INPUT_PGCOMMON_SQL_UTILS_HPP
