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

#include "process_csv_file.hpp"
#include "../../plugins/input/csv/csv_getline.hpp"
#include "../../plugins/input/csv/csv_utils.hpp"
#include <mapnik/datasource.hpp>
#include <mapnik/geometry/envelope.hpp>
#include <mapnik/util/utf_conv_win.hpp>

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#pragma GCC diagnostic pop
#include <mapnik/mapped_memory_cache.hpp>
#endif

#include <fstream>
#include <iostream>
#include <sstream>

namespace mapnik { namespace detail {

template <typename T>
std::pair<bool,typename T::value_type::first_type> process_csv_file(T & boxes, std::string const& filename, std::string const& manual_headers, char separator, char quote)
{
    using box_type = typename T::value_type::first_type;
    csv_utils::csv_file_parser p;
    p.manual_headers_ = manual_headers;
    p.separator_ = separator;
    p.quote_ = quote;

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
    using file_source_type = boost::interprocess::ibufferstream;
    file_source_type csv_file;
    mapnik::mapped_region_ptr mapped_region;
    boost::optional<mapnik::mapped_region_ptr> memory =
        mapnik::mapped_memory_cache::instance().find(filename, true);
    if (memory)
    {
        mapped_region = *memory;
        csv_file.buffer(static_cast<char*>(mapped_region->get_address()),mapped_region->get_size());
    }
    else
    {
        std::clog << "Error : cannot mmap " << filename << std::endl;
        return std::make_pair(false, box_type(p.extent_));
    }
#else
 #if defined(_WINDOWS)
    std::ifstream csv_file(mapnik::utf8_to_utf16(filename),std::ios_base::in | std::ios_base::binary);
 #else
    std::ifstream csv_file(filename.c_str(),std::ios_base::in | std::ios_base::binary);
 #endif
    if (!csv_file.is_open())
    {
        std::clog << "Error : cannot open " << filename << std::endl;
        return std::make_pair(false, box_type(p.extent_));
    }
#endif
    try
    {
        p.parse_csv_and_boxes(csv_file, boxes);
        return std::make_pair(true, box_type(p.extent_));
    }
    catch (std::exception const& ex)
    {
        std::clog << ex.what() << std::endl;
        return std::make_pair(false, box_type(p.extent_));
    }
}

using box_type = mapnik::box2d<float>;
using item_type = std::pair<box_type, std::pair<std::size_t, std::size_t>>;
using boxes_type = std::vector<item_type>;
template std::pair<bool,box_type> process_csv_file(boxes_type&, std::string const&, std::string const&, char, char);

}}
