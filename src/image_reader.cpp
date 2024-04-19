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
#include <mapnik/image_reader.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/factory.hpp>

namespace mapnik {

inline std::optional<std::string> type_from_bytes(char const* data, size_t size)
{
    unsigned char const* header = reinterpret_cast<unsigned char const*>(data);
    if (size >= 4)
    {
        unsigned int magic = (header[0] << 24) | (header[1] << 16) | (header[2] << 8) | header[3];
        if (magic == 0x89504E47U)
        {
            return "png";
        }
        else if (magic == 0x49492A00U || magic == 0x4D4D002AU)
        {
            return "tiff";
        }
    }
    if (size >= 2)
    {
        unsigned int magic = ((header[0] << 8) | header[1]) & 0xffff;
        if (magic == 0xffd8)
        {
            return "jpeg";
        }
    }

    if (size >= 12)
    {
        if (header[0] == 'R' && header[1] == 'I' && header[2] == 'F' && header[3] == 'F' && header[8] == 'W' &&
            header[9] == 'E' && header[10] == 'B' && header[11] == 'P')
        {
            return "webp";
        }
    }
    return std::nullopt;
}

image_reader* get_image_reader(char const* data, size_t size)
{
    const auto type = type_from_bytes(data, size);
    if (type.has_value())
        return factory<image_reader, std::string, char const*, size_t>::instance().create_object(*type, data, size);
    else
        throw image_reader_exception("image_reader: can't determine type from input data");
}

image_reader* get_image_reader(std::string const& filename, std::string const& type)
{
    return factory<image_reader, std::string, std::string const&>::instance().create_object(type, filename);
}

image_reader* get_image_reader(std::string const& filename)
{
    const auto type = type_from_filename(filename);
    if (type.has_value())
    {
        return factory<image_reader, std::string, std::string const&>::instance().create_object(*type, filename);
    }
    else
    {
        throw image_reader_exception("image_reader: can't determine type from input data");
    }
}

} // namespace mapnik
