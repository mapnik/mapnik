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
#include <mapnik/global.hpp> // for M_PI on windows
#include <mapnik/enumeration.hpp>
#include <mapnik/geometry.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/optional.hpp>
#pragma GCC diagnostic pop

// stl
#include <cmath>

namespace mapnik {

enum well_known_srs_enum : std::uint8_t {
    WGS_84,
    G_MERC,
    well_known_srs_enum_MAX
};

DEFINE_ENUM( well_known_srs_e, well_known_srs_enum );

static const double EARTH_RADIUS = 6378137.0;
static const double EARTH_DIAMETER = EARTH_RADIUS * 2.0;
static const double EARTH_CIRCUMFERENCE = EARTH_DIAMETER * M_PI;
static const double MAXEXTENT = EARTH_CIRCUMFERENCE / 2.0;
static const double M_PI_by2 = M_PI / 2;
static const double D2R = M_PI / 180;
static const double R2D = 180 / M_PI;
static const double M_PIby360 = M_PI / 360;
static const double MAXEXTENTby180 = MAXEXTENT / 180;
static const double MAX_LATITUDE = R2D * (2 * std::atan(std::exp(180 * D2R)) - M_PI_by2);
static const std::string MAPNIK_LONGLAT_PROJ = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";
static const std::string MAPNIK_GMERC_PROJ = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0.0 +k=1.0 +units=m +nadgrids=@null +wktext +no_defs +over";

boost::optional<well_known_srs_e> is_well_known_srs(std::string const& srs);

boost::optional<bool> is_known_geographic(std::string const& srs);

static inline bool lonlat2merc(double * x, double * y , int point_count)
{
    for(int i=0; i<point_count; ++i)
    {
        if (x[i] > 180) x[i] = 180;
        else if (x[i] < -180) x[i] = -180;
        if (y[i] > MAX_LATITUDE) y[i] = MAX_LATITUDE;
        else if (y[i] < -MAX_LATITUDE) y[i] = -MAX_LATITUDE;
        x[i] = x[i] * MAXEXTENTby180;
        y[i] = std::log(std::tan((90 + y[i]) * M_PIby360)) * R2D;
        y[i] = y[i] * MAXEXTENTby180;
    }
    return true;
}

static inline bool merc2lonlat(double * x, double * y , int point_count)
{
    for(int i=0; i<point_count; i++)
    {
        if (x[i] > MAXEXTENT) x[i] = MAXEXTENT;
        else if (x[i] < -MAXEXTENT) x[i] = -MAXEXTENT;
        if (y[i] > MAXEXTENT) y[i] = MAXEXTENT;
        else if (y[i] < -MAXEXTENT) y[i] = -MAXEXTENT;
        x[i] = (x[i] / MAXEXTENT) * 180;
        y[i] = (y[i] / MAXEXTENT) * 180;
        y[i] = R2D * (2 * std::atan(std::exp(y[i] * D2R)) - M_PI_by2);
    }
    return true;
}

static inline bool lonlat2merc(geometry::line_string<double> & ls)
{
    for(auto & p : ls)
    {
        if (p.x > 180) p.x = 180;
        else if (p.x < -180) p.x = -180;
        if (p.y > MAX_LATITUDE) p.y = MAX_LATITUDE;
        else if (p.y < -MAX_LATITUDE) p.y = -MAX_LATITUDE;
        p.x = p.x * MAXEXTENTby180;
        p.y = std::log(std::tan((90 + p.y) * M_PIby360)) * R2D;
        p.y = p.y * MAXEXTENTby180;
    }
    return true;
}

static inline bool merc2lonlat(geometry::line_string<double> & ls)
{
    for (auto & p : ls)
    {
        if (p.x > MAXEXTENT) p.x = MAXEXTENT;
        else if (p.x < -MAXEXTENT) p.x = -MAXEXTENT;
        if (p.y > MAXEXTENT) p.y = MAXEXTENT;
        else if (p.y < -MAXEXTENT) p.y = -MAXEXTENT;
        p.x = (p.x / MAXEXTENT) * 180;
        p.y = (p.y / MAXEXTENT) * 180;
        p.y = R2D * (2 * std::atan(std::exp(p.y * D2R)) - M_PI_by2);
    }
    return true;
}

}

#endif // MAPNIK_WELL_KNOWN_SRS_HPP
