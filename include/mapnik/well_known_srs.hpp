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

#ifndef MAPNIK_WELL_KNOWN_SRS_HPP
#define MAPNIK_WELL_KNOWN_SRS_HPP

// mapnik
#include <mapnik/enumeration.hpp>
#include <mapnik/geometry/point.hpp>
#include <mapnik/util/math.hpp>

#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/optional.hpp>
MAPNIK_DISABLE_WARNING_POP

// stl
#include <vector>

namespace mapnik {

enum well_known_srs_enum : std::uint8_t {
    WGS_84,
    WEB_MERC,
    well_known_srs_enum_MAX
};

DEFINE_ENUM( well_known_srs_e, well_known_srs_enum );

constexpr double EARTH_RADIUS = 6378137.0;
constexpr double EARTH_CIRCUMFERENCE = EARTH_RADIUS * util::tau;
constexpr double MERC_MAX_EXTENT = EARTH_RADIUS * util::pi;
constexpr double MERC_MAX_LATITUDE = 85.0511287798065923778;
              // MERC_MAX_LATITUDE = degrees(2 * atan(exp(pi)) - pi / 2)

extern MAPNIK_DECL std::string const MAPNIK_GEOGRAPHIC_PROJ;
extern MAPNIK_DECL std::string const MAPNIK_WEBMERCATOR_PROJ;

MAPNIK_DECL boost::optional<bool> is_known_geographic(std::string const& srs);
MAPNIK_DECL boost::optional<well_known_srs_e> is_well_known_srs(std::string const& srs);

MAPNIK_DECL bool lonlat2merc(double & x, double & y);
MAPNIK_DECL bool lonlat2merc(double * x, double * y, std::size_t point_count,
                                                     std::size_t stride = 1);
MAPNIK_DECL bool lonlat2merc(std::vector<geometry::point<double>> & ls);

MAPNIK_DECL bool merc2lonlat(double & x, double & y);
MAPNIK_DECL bool merc2lonlat(double * x, double * y, std::size_t point_count,
                                                     std::size_t stride = 1);
MAPNIK_DECL bool merc2lonlat(std::vector<geometry::point<double>> & ls);

}

#endif // MAPNIK_WELL_KNOWN_SRS_HPP
