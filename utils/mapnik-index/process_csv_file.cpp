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

#include "process_csv_file.hpp"
#include "../../plugins/input/csv/csv_getline.hpp"
#include "../../plugins/input/csv/csv_utils.hpp"
#include <mapnik/datasource.hpp>
#include <mapnik/geometry/envelope.hpp>
#include <mapnik/util/utf_conv_win.hpp>

#if defined(MAPNIK_MEMORY_MAPPED_FILE)
#include <boost/algorithm/string.hpp>
#endif
#include <mapnik/util/mapped_memory_file.hpp>

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


    util::mapped_memory_file csv_file{filename};
    try
    {
        p.parse_csv_and_boxes(csv_file.file(), boxes);
        return std::make_pair(true, box_type(p.extent_));
    }
    catch (std::exception const& ex)
    {
        std::clog << ex.what() << std::endl;
        return std::make_pair(false, box_type(p.extent_));
    }
}

using box_type = mapnik::box2d<float>;
using item_type = std::pair<box_type, std::pair<std::uint64_t, std::uint64_t>>;
using boxes_type = std::vector<item_type>;
template std::pair<bool,box_type> process_csv_file(boxes_type&, std::string const&, std::string const&, char, char);

}}
