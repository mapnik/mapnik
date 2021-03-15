/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2021 Artem Pavlenko
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

// mapnik
#include <mapnik/well_known_srs.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/enumeration.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/optional.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <cmath>

namespace mapnik {

extern std::string const MAPNIK_GEOGRAPHIC_PROJ =
    "epsg:4326";  //wgs84

extern std::string const MAPNIK_WEBMERCATOR_PROJ =
    "epsg:3857"; // webmercator

static const char * well_known_srs_strings[] = {
    MAPNIK_GEOGRAPHIC_PROJ.c_str(),
    MAPNIK_WEBMERCATOR_PROJ.c_str(),
    ""
};

boost::optional<well_known_srs_e> is_well_known_srs(std::string const& srs)
{
    if (srs == MAPNIK_GEOGRAPHIC_PROJ)
    {
        return boost::optional<well_known_srs_e>(mapnik::WGS_84);
    }
    else if (srs == MAPNIK_WEBMERCATOR_PROJ)
    {
        return boost::optional<well_known_srs_e>(mapnik::WEB_MERC);
    }
    return boost::optional<well_known_srs_e>();
}

boost::optional<bool> is_known_geographic(std::string const& srs)
{
    std::string trimmed = util::trim_copy(srs);
    if (trimmed == MAPNIK_GEOGRAPHIC_PROJ)
    {
        return boost::optional<bool>(true);
    }
    else if (trimmed ==  MAPNIK_WEBMERCATOR_PROJ)
    {
        return boost::optional<bool>(true);
    }
    return boost::optional<bool>();
}

IMPLEMENT_ENUM( well_known_srs_e, well_known_srs_strings )

bool lonlat2merc(double & x, double & y)
{
    using namespace util;
    auto dx = clamp(x, -180.0, 180.0);
    auto dy = clamp(y, -MERC_MAX_LATITUDE, MERC_MAX_LATITUDE);
    x = EARTH_RADIUS * radians(dx);
    y = EARTH_RADIUS * std::log(std::tan(radians(90.0 + dy) / 2.0));
    return true;
}

bool lonlat2merc(double * x, double * y, std::size_t point_count, std::size_t stride)
{
    for (std::size_t i = 0; i < point_count; ++i)
    {
        lonlat2merc(x[i * stride], y[i * stride]);
    }
    return true;
}

bool lonlat2merc(std::vector<geometry::point<double>> & ls)
{
    for (auto & p : ls)
    {
        lonlat2merc(p.x, p.y);
    }
    return true;
}

bool merc2lonlat(double & x, double & y)
{
    using namespace util;
    auto rx = clamp(x / EARTH_RADIUS, -pi, pi);
    auto ry = clamp(y / EARTH_RADIUS, -pi, pi);
    x = degrees(rx);
    y = degrees(2.0 * std::atan(std::exp(ry)) - pi / 2.0);
    return true;
}

bool merc2lonlat(double * x, double * y, std::size_t point_count, std::size_t stride)
{
    for(std::size_t i = 0; i < point_count; ++i)
    {
        merc2lonlat(x[i * stride], y[i * stride]);
    }
    return true;
}

bool merc2lonlat(std::vector<geometry::point<double>> & ls)
{
    for (auto & p : ls)
    {
        merc2lonlat(p.x, p.y);
    }
    return true;
}

}
