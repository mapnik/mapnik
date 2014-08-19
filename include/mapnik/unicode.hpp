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

#ifndef MAPNIK_UNICODE_HPP
#define MAPNIK_UNICODE_HPP

//mapnik
#include <mapnik/config.hpp>
#include <mapnik/noncopyable.hpp>

// icu
#include <mapnik/value_types.hpp>
#include <unicode/ucnv.h>

// stl
#include <cstdint>
#include <string>

namespace mapnik {

class MAPNIK_DECL transcoder : private mapnik::noncopyable
{
public:
    explicit transcoder (std::string const& encoding);
    mapnik::value_unicode_string transcode(const char* data, std::int32_t length = -1) const;
    ~transcoder();
private:
    bool ok_;
    UConverter * conv_;
};
}

namespace U_ICU_NAMESPACE {

inline std::size_t hash_value(mapnik::value_unicode_string const& val)
{
    return val.hashCode();
}

}

#endif // MAPNIK_UNICODE_HPP
