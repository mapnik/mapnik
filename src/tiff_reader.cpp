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
#include <mapnik/debug.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/util/char_array_buffer.hpp>
extern "C" {
#include <tiffio.h>
}

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#include <mapnik/warning.hpp>
MAPNIK_DISABLE_WARNING_PUSH
#include <mapnik/warning_ignore.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
MAPNIK_DISABLE_WARNING_POP
#include <mapnik/mapped_memory_cache.hpp>
#endif
#include "tiff_reader.hpp"

// stl
#include <memory>
#include <fstream>
#include <algorithm>

namespace mapnik {
namespace detail {

toff_t tiff_seek_proc(thandle_t handle, toff_t off, int whence)
{
    std::istream* in = reinterpret_cast<std::istream*>(handle);

    switch (whence)
    {
        case SEEK_SET:
            in->seekg(off, std::ios_base::beg);
            break;
        case SEEK_CUR:
            in->seekg(off, std::ios_base::cur);
            break;
        case SEEK_END:
            in->seekg(off, std::ios_base::end);
            break;
    }
    return static_cast<toff_t>(in->tellg());
}

int tiff_close_proc(thandle_t)
{
    return 0;
}

toff_t tiff_size_proc(thandle_t handle)
{
    std::istream* in = reinterpret_cast<std::istream*>(handle);
    std::ios::pos_type pos = in->tellg();
    in->seekg(0, std::ios::end);
    std::ios::pos_type len = in->tellg();
    in->seekg(pos);
    return static_cast<toff_t>(len);
}

tsize_t tiff_read_proc(thandle_t handle, tdata_t buf, tsize_t size)
{
    std::istream* in = reinterpret_cast<std::istream*>(handle);
    std::streamsize request_size = size;
    if (static_cast<tsize_t>(request_size) != size)
        return static_cast<tsize_t>(-1);
    in->read(reinterpret_cast<char*>(buf), request_size);
    return static_cast<tsize_t>(in->gcount());
}

tsize_t tiff_write_proc(thandle_t, tdata_t, tsize_t)
{
    return 0;
}

void tiff_unmap_proc(thandle_t, tdata_t, toff_t) {}

int tiff_map_proc(thandle_t, tdata_t*, toff_t*)
{
    return 0;
}
} // namespace detail

image_reader* create_tiff_reader(std::string const& filename)
{
#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    return new tiff_reader<boost::interprocess::ibufferstream>(filename);
#else
    return new tiff_reader<std::filebuf>(filename);
#endif
}

image_reader* create_tiff_reader2(char const* data, std::size_t size)
{
    return new tiff_reader<mapnik::util::char_array_buffer>(data, size);
}

void register_tiff_reader()
{
    const bool registered = register_image_reader("tiff", create_tiff_reader);
    const bool registered2 = register_image_reader("tiff", create_tiff_reader2);
}

} // namespace mapnik
