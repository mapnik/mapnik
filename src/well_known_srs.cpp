/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2025 Artem Pavlenko
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

// stl
#include <cmath>
#include <optional>

namespace mapnik {
constexpr const char MAPNIK_GEOGRAPHIC_PROJ_STR[10]{"epsg:4326"};
extern std::string const MAPNIK_GEOGRAPHIC_PROJ = MAPNIK_GEOGRAPHIC_PROJ_STR; // wgs84

constexpr const char MAPNIK_WEBMERCATOR_PROJ_STR[10]{"epsg:3857"};
extern std::string const MAPNIK_WEBMERCATOR_PROJ = MAPNIK_WEBMERCATOR_PROJ_STR; // webmercator

static const char* well_known_srs_strings[] = {MAPNIK_GEOGRAPHIC_PROJ.c_str(), MAPNIK_WEBMERCATOR_PROJ.c_str(), ""};

std::optional<well_known_srs_e> is_well_known_srs(std::string const& srs)
{
    if (srs == MAPNIK_GEOGRAPHIC_PROJ)
    {
        return mapnik::well_known_srs_enum::WGS_84;
    }
    else if (srs == MAPNIK_WEBMERCATOR_PROJ)
    {
        return mapnik::well_known_srs_enum::WEB_MERC;
    }
    return std::nullopt;
}

std::optional<bool> is_known_geographic(std::string const& srs)
{
    std::string trimmed = util::trim_copy(srs);
    if (trimmed == MAPNIK_GEOGRAPHIC_PROJ)
    {
        return true;
    }
    else if (trimmed == MAPNIK_WEBMERCATOR_PROJ)
    {
        return false;
    }
    return std::nullopt;
}

using well_known_srs_e_str = mapnik::detail::EnumStringT<well_known_srs_enum>;
constexpr detail::EnumMapT<well_known_srs_enum, 3> well_known_srs_e_map{{
  well_known_srs_e_str{well_known_srs_enum::WGS_84, MAPNIK_GEOGRAPHIC_PROJ_STR},
  well_known_srs_e_str{well_known_srs_enum::WEB_MERC, MAPNIK_WEBMERCATOR_PROJ_STR},
  well_known_srs_e_str{well_known_srs_enum::well_known_srs_enum_MAX, ""},
}};
IMPLEMENT_ENUM(well_known_srs_e, well_known_srs_enum)

bool lonlat2merc(double& x, double& y)
{
    using namespace util;
    auto dx = clamp(x, -180.0, 180.0);
    auto dy = clamp(y, -MERC_MAX_LATITUDE, MERC_MAX_LATITUDE);
    x = EARTH_RADIUS * radians(dx);
    y = EARTH_RADIUS * std::log(std::tan(radians(90.0 + dy) / 2.0));
    return true;
}

bool lonlat2merc(double* x, double* y, std::size_t point_count, std::size_t stride)
{
    for (std::size_t i = 0; i < point_count; ++i)
    {
        lonlat2merc(x[i * stride], y[i * stride]);
    }
    return true;
}

bool lonlat2merc(std::vector<geometry::point<double>>& ls)
{
    for (auto& p : ls)
    {
        lonlat2merc(p.x, p.y);
    }
    return true;
}

bool merc2lonlat(double& x, double& y)
{
    using namespace util;
    auto rx = clamp(x / EARTH_RADIUS, -pi, pi);
    auto ry = clamp(y / EARTH_RADIUS, -pi, pi);
    x = degrees(rx);
    y = degrees(2.0 * std::atan(std::exp(ry)) - pi / 2.0);
    return true;
}

bool merc2lonlat(double* x, double* y, std::size_t point_count, std::size_t stride)
{
    for (std::size_t i = 0; i < point_count; ++i)
    {
        merc2lonlat(x[i * stride], y[i * stride]);
    }
    return true;
}

bool merc2lonlat(std::vector<geometry::point<double>>& ls)
{
    for (auto& p : ls)
    {
        merc2lonlat(p.x, p.y);
    }
    return true;
}

} // namespace mapnik
