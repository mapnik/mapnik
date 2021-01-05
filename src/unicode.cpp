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
#include <mapnik/unicode.hpp>
#include <mapnik/value_types.hpp>

// std
#include <stdexcept>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <unicode/ucnv.h>
#pragma GCC diagnostic pop

namespace mapnik {

transcoder::transcoder (std::string const& encoding)
    : conv_(0)
{
    UErrorCode err = U_ZERO_ERROR;
    conv_ = ucnv_open(encoding.c_str(),&err);
    if (!U_SUCCESS(err))
    {
        // NOTE: conv_ should be null on error so no need to call ucnv_close
        throw std::runtime_error(std::string("could not create converter for ") + encoding);
    }
}

mapnik::value_unicode_string transcoder::transcode(const char* data, std::int32_t length) const
{
    UErrorCode err = U_ZERO_ERROR;

    mapnik::value_unicode_string ustr(data,length,conv_,err);
    if (ustr.isBogus())
    {
        ustr.remove();
    }
    return ustr;
}

transcoder::~transcoder()
{
    if (conv_) ucnv_close(conv_);
}


void to_utf8(mapnik::value_unicode_string const& input, std::string & target)
{
    target.clear(); // mimic previous target.assign(...) semantics
    input.toUTF8String(target); // this appends to target
}

}
