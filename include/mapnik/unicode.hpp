/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2017 Artem Pavlenko
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
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/value/types.hpp>

// std
#include <cstdint>
#include <string>
// icu
#if (U_ICU_VERSION_MAJOR_NUM >= 59)
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <unicode/unistr.h>
#pragma GCC diagnostic pop
#endif

struct UConverter;

namespace mapnik {

class MAPNIK_DECL transcoder : private util::noncopyable
{
public:
    explicit transcoder (std::string const& encoding);
    ~transcoder();

    value_unicode_string transcode(char const* data, std::int32_t length = -1) const;
    value_unicode_string operator() (char const* data, std::int32_t length = -1) const;
    value_unicode_string operator() (std::string const& str) const;

private:
    UConverter * conv_;
};

// convenience functions
std::string MAPNIK_DECL to_utf8(value_unicode_string const& input);
void MAPNIK_DECL to_utf8(value_unicode_string const& input, std::string & target);

}

#endif // MAPNIK_UNICODE_HPP
