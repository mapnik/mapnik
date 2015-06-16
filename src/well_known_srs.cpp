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

// mapnik
#include <mapnik/well_known_srs.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/enumeration.hpp>

// boost
#include <boost/optional.hpp>

namespace mapnik {

static const char * well_known_srs_strings[] = {
    "mapnik-longlat",
    "mapnik-gmerc",
    ""
};

boost::optional<well_known_srs_e> is_well_known_srs(std::string const& srs)
{
    if (srs == "+init=epsg:4326" || srs == MAPNIK_LONGLAT_PROJ)
    {
        return boost::optional<well_known_srs_e>(mapnik::WGS_84);
    }
    else if (srs == "+init=epsg:3857" || srs == MAPNIK_GMERC_PROJ)
    {
        return boost::optional<well_known_srs_e>(mapnik::G_MERC);
    }
    return boost::optional<well_known_srs_e>();
}

boost::optional<bool> is_known_geographic(std::string const& srs)
{
    std::string trimmed = util::trim_copy(srs);
    if (trimmed == "+init=epsg:3857")
    {
        return boost::optional<bool>(false);
    }
    else if (trimmed == "+init=epsg:4326")
    {
        return boost::optional<bool>(true);
    }
    else if (srs.find("+proj=") != std::string::npos)
    {
        if ((srs.find("+proj=longlat") != std::string::npos) ||
                          (srs.find("+proj=latlong") != std::string::npos) ||
                          (srs.find("+proj=lonlat") != std::string::npos) ||
                          (srs.find("+proj=latlon") != std::string::npos)
                         )
        {
            return boost::optional<bool>(true);
        }
        else
        {
            return boost::optional<bool>(false);
        }
    }
    return boost::optional<bool>();
}

IMPLEMENT_ENUM( well_known_srs_e, well_known_srs_strings )

}
