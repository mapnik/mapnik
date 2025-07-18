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

#ifndef MAPNIK_UNICODE_HPP
#define MAPNIK_UNICODE_HPP

// mapnik
#include <mapnik/config.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/value/types.hpp>

// std
#include <cstdint>
#include <string>
// icu
#if (U_ICU_VERSION_MAJOR_NUM >= 59)
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <unicode/unistr.h>
MAPNIK_DISABLE_WARNING_POP
#endif

struct UConverter;

namespace mapnik {

class MAPNIK_DECL transcoder : private util::noncopyable
{
  public:
    explicit transcoder(std::string const& encoding);
    mapnik::value_unicode_string transcode(char const* data, std::int32_t length = -1) const;
    ~transcoder();

  private:
    UConverter* conv_;
};

// convinience method
void MAPNIK_DECL to_utf8(mapnik::value_unicode_string const& input, std::string& target);

} // namespace mapnik

#endif // MAPNIK_UNICODE_HPP
