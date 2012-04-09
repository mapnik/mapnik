/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/debug.hpp>

// stl
#include <ctime>

#ifndef MAPNIK_LOG_FORMAT
#define MAPNIK_LOG_FORMAT "Mapnik LOG> %Y-%m-%d %H:%M:%S:"
#endif

namespace mapnik { namespace logger {

// severity

severity::type severity::severity_level_ =
#ifdef MAPNIK_DEBUG
    severity::debug
#else
    severity::error
#endif
;

severity::severity_map severity::object_severity_level_ = severity::severity_map();


// format

#define __xstr__(s) __str__(s)
#define __str__(s) #s

std::string format::format_ = __xstr__(MAPNIK_LOG_FORMAT);

#undef __xstr__
#undef __str__

std::string format::str()
{
    char buf[256];
    const time_t tm = time(0);
    strftime(buf, sizeof(buf), format::format_.c_str(), localtime(&tm));
    return buf;
}


}
}
